#ifndef COMMAND_INTERPRETER_H
#define COMMAND_INTERPRETER_H

#include <Arduino.h>
#include <functional>
#include <map>
#include <IPeripheral.h>

namespace VMCBuddy {

/**
 * UART line-based command dispatcher.
 *
 * Reads newline-delimited commands from a Stream (HardwareSerial,
 * MDB, or SoftwareSerial), tokenizes to `CMD ARG1 ARG2 ...`,
 * and dispatches to registered handler functions.
 *
 * Matches commands case-insensitively. Unknown commands echo
 * `UNKNOWN:` back to the stream.
 *
 * Runs as a Task on the cooperative scheduler (10 ms poll interval).
 *
 * Usage:
 *   CommandInterpreter cmd(&Serial, &scheduler);
 *   cmd.on("STATUS", [](const char* args){ Serial.println("OK"); });
 *   cmd.on("PULSE",  [](const char* args){ ... });
 *   board.addPeripheral(&cmd);
 */
class CommandInterpreter : public IPeripheral {
public:
    using CmdHandler = std::function<void(const char* args)>;

    explicit CommandInterpreter(Stream* stream);

    bool begin() override;
    void end() override;
    const char* name() const override { return "CmdInterp"; }

    bool on(const char* command, CmdHandler handler);
    void off(const char* command);

private:
    Stream* _stream;
    std::map<String, CmdHandler> _handlers;
    String _lineBuf;

    bool tick();
    void dispatch(const String& line);
};

inline CommandInterpreter::CommandInterpreter(Stream* stream)
    : IPeripheral(), _stream(stream) {
    setInterval(10);
    setIterations(TASK_FOREVER);
    setCallback([this](){ if(_active) tick(); });
}

inline bool CommandInterpreter::begin() { Task::enable(); _active = true; return true; }
inline void CommandInterpreter::end() { Task::disable(); _active = false; }

inline bool CommandInterpreter::on(const char* command, CmdHandler handler) {
    _handlers[String(command)] = handler;
    return true;
}

inline void CommandInterpreter::off(const char* command) {
    _handlers.erase(String(command));
}

inline bool CommandInterpreter::tick() {
    while (_stream->available()) {
        char c = _stream->read();
        if (c == '\n' || c == '\r') {
            if (_lineBuf.length() > 0) { dispatch(_lineBuf); _lineBuf = ""; }
        } else {
            _lineBuf += c;
            if (_lineBuf.length() >= 128) _lineBuf = "";
        }
    }
    return true;
}

inline void CommandInterpreter::dispatch(const String& line) {
    int space = line.indexOf(' ');
    String cmd = space > 0 ? line.substring(0, space) : line;
    cmd.toUpperCase();
    auto it = _handlers.find(cmd);
    if (it != _handlers.end()) {
        it->second(space > 0 ? line.c_str() + space + 1 : "");
    } else {
        _stream->print("UNKNOWN: "); _stream->println(line);
    }
}

} // namespace VMCBuddy
#endif // COMMAND_INTERPRETER_H
