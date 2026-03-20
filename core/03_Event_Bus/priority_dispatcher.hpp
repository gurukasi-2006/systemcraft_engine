#pragma once

#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>
#include <algorithm>
#include "event_type_registry.hpp"

/**
 * @file priority_dispatcher.hpp
 * @brief Routes events synchronously, guaranteeing high-priority listeners execute first.
 */

class IPriorityContainer {
public:
    virtual ~IPriorityContainer() = default;
};

template<typename T>
class PriorityContainer : public IPriorityContainer {
public:
    std::vector<std::pair<int, std::function<void(const T&)>>> callbacks;

    bool needs_sorting = false;
};

class PriorityDispatcher {
private:
    std::unordered_map<EventID, std::unique_ptr<IPriorityContainer>> buffers;

public:
    /**
     * @brief Subscribes a callback to an event type with a specific execution priority.
     * @tparam T The specific event struct.
     * @param priority Lower numbers run FIRST (e.g., 0 runs before 10).
     * @param callback The function to execute.
     */
    template<typename T>
    void subscribe(int priority, std::function<void(const T&)> callback) {
        EventID id = EventTypeRegistry::getID<T>();

        if (buffers.find(id) == buffers.end()) {
            buffers[id] = std::make_unique<PriorityContainer<T>>();
        }

        auto* container = static_cast<PriorityContainer<T>*>(buffers[id].get());
        container->callbacks.push_back({priority, callback});
        container->needs_sorting = true;
    }

    /**
     * @brief Immediately dispatches the event, respecting the priority of all listeners.
     * @tparam T The specific event struct.
     */
    template<typename T>
    void dispatch(const T& event) {
        EventID id = EventTypeRegistry::getID<T>();
        if (buffers.find(id) == buffers.end()) return;

        auto* container = static_cast<PriorityContainer<T>*>(buffers[id].get());


        if (container->needs_sorting) {
            std::sort(container->callbacks.begin(), container->callbacks.end(),
                [](const auto& a, const auto& b) {
                    return a.first < b.first;
                });
            container->needs_sorting = false;
        }

        for (const auto& pair : container->callbacks) {
            pair.second(event);
        }
    }
};