#pragma once
#include "glm/glm.hpp"
#include <any>

using namespace glm;

class EventBus {
public:
    template <typename T>
    void emit(const T& event) { events.push_back(std::make_any<T>(event)); }

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
};
