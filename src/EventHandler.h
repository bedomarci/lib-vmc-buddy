#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include <algorithm>
#include <vector>

namespace VMCBuddy {

enum class Event {
    BUTTON_PRESS,
    BUTTON_RELEASE,
    PULSE_INT,
    CHANNEL_START,
    CHANNEL_STOP,
    CHANNEL_TICK,
    LED_START,
    LED_STOP,
    LED_TICK,
};

class EventHandler {
public:
    using EventCallback = void(*)(int, int, int);
    using tHandler = std::pair<Event, EventCallback>;
    using tHandlerContainer = std::vector<tHandler>;

    static void subscribe(Event event, EventCallback handler) {
        getHandlers().emplace_back(event, handler);
    }

    static void unsubscribe(EventCallback handler) {
        auto& handlers = getHandlers();
        handlers.erase(
            std::remove_if(handlers.begin(), handlers.end(), [&](tHandler& entry) {
                return entry.second == handler;
            }),
            handlers.end());
    }

    static void trigger(Event event, int param1 = 0, int param2 = 0, int param3 = 0) {
        auto& handlers = getHandlers();
        for (tHandler handler : handlers) {
            if (handler.first == event)
                handler.second(param1, param2, param3);
        }
    }

private:
    static tHandlerContainer& getHandlers() {
        static tHandlerContainer handlers;
        return handlers;
    }
};

} // VMCBuddy
#endif // EVENT_BUS_H