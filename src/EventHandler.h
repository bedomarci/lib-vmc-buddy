#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include <algorithm>
#include <vector>

namespace VMCBuddy
{
    // Define an Event enum
    enum class Event
    {
        BUTTON_PRESS,
        BUTTON_RELEASE,
        PULSE_INT,
    };

    class EventHandler
    {
    public:
        using EventCallback = void(*)(int, int, int); // Raw function pointer type for handlers
        using tHandler  = std::pair<Event, EventCallback>;
        using tHandlerContainer  = std::vector<tHandler>;


        // Subscribe to an event
        static void subscribe(Event event, EventCallback handler)
        {
            getHandlers().emplace_back(event, handler);
        }

        // Unsubscribe from a callback
        static void unsubscribe(EventCallback handler)
        {
            auto& handlers = getHandlers();
            handlers.erase(
                std::remove_if(handlers.begin(), handlers.end(), [&](tHandler& entry)
                {
                    return entry.second == handler;
                }),
                handlers.end()
            );
        }

        // Trigger an event
        static void trigger(Event event, int param1 = 0, int param2 = 0, int param3 = 0)
        {
            auto& handlers = getHandlers();
            for (tHandler handler : handlers)
            {
                if (handler.first == event)
                {
                    handler.second(param1, param2, param3);
                }
            }
        }

    private:
        // Get the static list of event handlers

        static tHandlerContainer& getHandlers()
        {
            static tHandlerContainer handlers;
            return handlers;
        }
    };
}

#endif // EVENT_BUS_H
