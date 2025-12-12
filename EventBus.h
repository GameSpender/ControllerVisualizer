#pragma once
#include "glm/glm.hpp"
#include <any>
#include <vector>
#include <unordered_map>
#include <functional>
#include <typeindex>

using namespace glm;

class EventBus {
public:

    // Subscribe a callback for a specific event type
    template <typename EventType>
    void subscribe(std::function<void(const EventType&)> callback) {
        auto& subs = subscribers[typeid(EventType)];
        // Store as std::function<void(std::any)>
        subs.push_back([callback](const std::any& e) {
            // Cast to EventType reference, not pointer
            callback(std::any_cast<const EventType&>(e));
            });
    }

    // Unsubscribe all callbacks for a type (optional)
    template <typename EventType>
    void clearSubscribers() {
        subscribers.erase(typeid(EventType));
    }

    // Emit an event, notify all subscribers
    template <typename EventType>
    void emit(const EventType& event) {
        // 1. Add to queued events
        events.push_back(std::make_any<EventType>(event));

        // 2. Notify subscribers immediately
        auto it = subscribers.find(typeid(EventType));
        if (it != subscribers.end()) {
            for (auto& sub : it->second) {
                sub(events.back());
            }
        }
    }

    template <typename T, typename F>
    void process(F func) {
        for (auto& ev : events) {
            if (auto* e = std::any_cast<T>(&ev)) {
                func(*e);
            }
        }
    }

    void clear() { events.clear(); }

private:
    std::vector<std::any> events;

    std::unordered_map<std::type_index, std::vector<std::function<void(const std::any&)>>> subscribers;
};
