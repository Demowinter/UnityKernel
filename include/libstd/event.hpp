#pragma once
#include <libstd/vector.hpp>
#include <libstd/function.hpp>

namespace STDLib {
    template<typename T>
    class EventChannel {
    public:
        using EventHandler = STDLib::Function<void(const T&)>;

        void subscribe(EventHandler handler) {
            subscribers.push_back(handler);
        }

        void publish(const T& data) {
            for (EventHandler& handler : subscribers) handler(data);
        }

    private:
        STDLib::Vector<EventHandler> subscribers;
    };
}