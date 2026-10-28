#ifndef MDB_H
#define MDB_H

#include <Arduino.h>
#include <ISerial.h>
#include <macros.h>
#include <UARTBus.h>

/**
 * GPIO bit-bang MDB/ICP 9-bit driver at 9600 baud.
 *
 * ESP32 hardware UART does not support 9-bit mode — full GPIO
 * bit-bang instead. TX is blocking busy-wait (~1.1 ms/byte).
 * RX uses a falling-edge ISR + FreeRTOS queue with ets_delay_us
 * timing (1 ms ISR hold per byte).
 *
 * Pins: BoardConfig.uart2Rx / uart2Tx (shared with RS-485 via
 * the 2×15 through-hole jumper field).
 *
 * Inherits ISerial → IPeripheral → Task for scheduler integration.
 *
 * Usage:
 *   MDB mdb(0x10);        // peripheral address 0x10
 *   mdb.begin();
 *   mdb.sendPacket(0x0B); // send POLL response
 */
#define MDB_BAUD         9600
#define MDB_BIT_US       104   // 1 / 9600 * 1e6
#define MDB_HALF_BIT_US  156   // 1.5 bit time for RX sampling midpoint
#define MDB_RX_QUEUE_LEN 16
#define BIT_MODE_SET     0x100 // 9th bit = address / checksum

namespace VMCBuddy {

class MDB : public ISerial {
public:
    explicit MDB(uint8_t peripheralAddr = 0x01);
    ~MDB() override;

    bool begin() override;
    void end() override;

    // — ISerial contract: standard 8-bit read/write on top of 9-bit transport —
    int available() override;  // items in RX queue
    int read() override;       // read9() with mode bit stripped
    int peek() override;       // not supported for queue-based RX
    size_t write(uint8_t c) override;
    size_t write(const uint8_t* buf, size_t len) override;
    void flush() override;

    const char* name() const override { return "MDB"; }

    // — MDB-specific 9-bit operations —
    /** Write a 9-bit frame. modeBit=1 for address/checksum, 0 for data. */
    void write9(uint8_t data, bool modeBit);

    /** Get next 9-bit word from RX queue. Returns (modeBit<<8)|data, or -1 if empty. */
    int read9();

    bool isRunning() const { return _active; }

    // — L2 packet framing —
    struct Packet {
        uint8_t addr;       uint8_t cmd;
        uint8_t data[64];   uint8_t dataLen;
        bool valid;
        void clear() { addr=cmd=0; dataLen=0; valid=false; }
    };
    Packet* readPacket(uint32_t timeoutMs = 5);
    void sendPacket(uint8_t cmd, const uint8_t* pdata = nullptr, uint8_t len = 0);

    uint8_t address() const { return _addr; }

private:
    uint8_t _addr, _txPin, _rxPin;
    QueueHandle_t _rxQueue;
    Packet _packet;

    static void _rxIsr(void* arg);
};

inline MDB::MDB(uint8_t peripheralAddr)
    : ISerial(), _addr(peripheralAddr), _txPin(0), _rxPin(0), _rxQueue(nullptr) {}

inline MDB::~MDB() { if (_active) end(); }

inline bool MDB::begin() {
    if (_active) return true;
    if (!UARTBus::acquire(2, UARTBus::MDB)) return false;
    const auto& cfg = Context::getInstance()->boardConfig;
    _txPin = cfg.uart2Tx;
    _rxPin = cfg.uart2Rx;

    pinMode(_txPin, OUTPUT);
    digitalWrite(_txPin, HIGH);
    pinMode(_rxPin, INPUT_PULLUP);

    _rxQueue = xQueueCreate(MDB_RX_QUEUE_LEN, sizeof(uint16_t));
    if (!_rxQueue) {
        UARTBus::release(2);
        return false;
    }

    esp_err_t isrService = gpio_install_isr_service(0);
    if (isrService != ESP_OK && isrService != ESP_ERR_INVALID_STATE) {
        vQueueDelete(_rxQueue);
        _rxQueue = nullptr;
        UARTBus::release(2);
        return false;
    }
    if (gpio_isr_handler_add((gpio_num_t)_rxPin, _rxIsr, this) != ESP_OK) {
        vQueueDelete(_rxQueue);
        _rxQueue = nullptr;
        UARTBus::release(2);
        return false;
    }
    if (gpio_set_intr_type((gpio_num_t)_rxPin, GPIO_INTR_NEGEDGE) != ESP_OK) {
        gpio_isr_handler_remove((gpio_num_t)_rxPin);
        vQueueDelete(_rxQueue);
        _rxQueue = nullptr;
        UARTBus::release(2);
        return false;
    }
    gpio_intr_enable((gpio_num_t)_rxPin);

    _active = true;
    return true;
}

inline void MDB::end() {
    if (!_active) return;
    gpio_intr_disable((gpio_num_t)_rxPin);
    gpio_isr_handler_remove((gpio_num_t)_rxPin);
    if (_rxQueue) { vQueueDelete(_rxQueue); _rxQueue = nullptr; }
    digitalWrite(_txPin, HIGH);
    UARTBus::release(2);
    _active = false;
}

// ——— L1: 9-bit TX (blocking busy-wait) ———

inline void MDB::write9(uint8_t data, bool modeBit) {
    if (!_active) return;
    uint16_t word = data | (modeBit ? BIT_MODE_SET : 0x0000);

    digitalWrite(_txPin, LOW);                // start
    ets_delay_us(MDB_BIT_US);
    for (uint8_t i = 0; i < 9; i++) {        // 9 bits LSB-first
        digitalWrite(_txPin, (word >> i) & 1);
        ets_delay_us(MDB_BIT_US);
    }
    digitalWrite(_txPin, HIGH);               // stop
    ets_delay_us(MDB_BIT_US);
}

inline void MDB::_rxIsr(void* arg) {
    MDB* self = (MDB*)arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    gpio_intr_disable((gpio_num_t)self->_rxPin);

    ets_delay_us(MDB_HALF_BIT_US);
    uint16_t word = 0;
    for (uint8_t i = 0; i < 9; i++) {
        word |= (gpio_get_level((gpio_num_t)self->_rxPin) ? 1 : 0) << i;
        ets_delay_us(MDB_BIT_US);
    }
    xQueueSendFromISR(self->_rxQueue, &word, &xHigherPriorityTaskWoken);
    gpio_intr_enable((gpio_num_t)self->_rxPin);
    if (xHigherPriorityTaskWoken == pdTRUE) portYIELD_FROM_ISR();
}

inline int MDB::available() {
    return _active && _rxQueue ? (int)uxQueueMessagesWaiting(_rxQueue) : 0;
}

inline int MDB::read9() {
    if (!_active || !_rxQueue) return -1;
    uint16_t word;
    return xQueueReceive(_rxQueue, &word, 0) == pdTRUE ? (int)word : -1;
}

// pony: strip mode bit for standard read(); peek() not feasible from FreeRTOS queue
inline int MDB::read() {
    int raw = read9();
    return raw >= 0 ? (int)(raw & 0xFF) : -1;
}

inline int MDB::peek() { return -1; }

inline size_t MDB::write(uint8_t c) {
    write9(c, false);
    return 1;
}

inline size_t MDB::write(const uint8_t* buf, size_t len) {
    if (!_active) return 0;
    for (size_t i = 0; i < len; i++) write9(buf[i], false);
    return len;
}

inline void MDB::flush() {}

// ——— L2: packet framing ———

inline MDB::Packet* MDB::readPacket(uint32_t timeoutMs) {
    _packet.clear();
    if (!_active) return &_packet;

    unsigned long start = millis();
    uint8_t checksum = 0;
    uint16_t w = 0;
    bool foundAddress = false;
    bool foundCommand = false;

    // Step 1: wait for address byte (mode bit = 1)
    while (millis() - start < timeoutMs) {
        int raw = read9(); if (raw < 0) continue;
        w = (uint16_t)raw;
        if (w & BIT_MODE_SET) {
            _packet.addr = w & 0xFF;
            checksum = _packet.addr;
            start = millis();
            foundAddress = true;
            break;
        }
    }
    if (!foundAddress) return &_packet;

    // Step 2: command byte (mode bit = 0)
    while (millis() - start < timeoutMs) {
        int raw = read9(); if (raw < 0) continue;
        w = (uint16_t)raw;
        if (!(w & BIT_MODE_SET)) {
            _packet.cmd = w & 0xFF;
            checksum += _packet.cmd;
            start = millis();
            foundCommand = true;
            break;
        }
    }
    if (!foundCommand) return &_packet;

    // Step 3: data bytes + checksum (mode bit = 1)
    uint8_t len = 0;
    while (millis() - start < timeoutMs) {
        int raw = read9(); if (raw < 0) continue;
        w = (uint16_t)raw;
        if (w & BIT_MODE_SET) {
            _packet.dataLen = len;
            _packet.valid   = (checksum == (w & 0xFF));
            return &_packet;
        }
        if (len < sizeof(_packet.data)) _packet.data[len++] = w & 0xFF;
        checksum += (w & 0xFF);
        start = millis();
    }
    _packet.valid = false;
    return &_packet;
}

inline void MDB::sendPacket(uint8_t cmd, const uint8_t* pdata, uint8_t len) {
    if (!_active) return;
    write9(_addr, true);
    write9(cmd, false);
    uint8_t chk = _addr + cmd;
    for (uint8_t i = 0; i < len; i++) { write9(pdata[i], false); chk += pdata[i]; }
    write9(chk, true);
    ets_delay_us(5000);
}

} // namespace VMCBuddy
#endif // MDB_H
