#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include "event_type_registry.hpp"

/**
 * @file event_publisher.hpp
 * @brief Allows systems to emit events into type-erased buffers for later processing.
 */

/**
 * @class IEventContainer
 * @brief The purely virtual base class that allows us to store different vectors in one map.
 */
class IEventContainer {
public:
    virtual ~IEventContainer() = default;
    virtual void clear() = 0;
};

/**
 * @class EventContainer
 * @brief The strictly-typed wrapper that actually holds the event data in memory.
 */
template<typename T>
class EventContainer : public IEventContainer {
public:
    std::vector<T> events;

    void clear() override {
        events.clear();
    }
};

/**
 * @class EventPublisher
 * @brief The global megaphone. Systems push events here, completely unaware of listeners.
 */
class EventPublisher {
private:
    std::unordered_map<EventID, std::unique_ptr<IEventContainer>> event_buffers;

public:
    /**
     * @brief Pushes an event into the master buffer to be processed later.
     * @tparam T The specific event struct.
     * @param event The actual data payload to store.
     */
    template<typename T>
    void publish(const T& event) {
        EventID id = EventTypeRegistry::getID<T>();

        if (event_buffers.find(id) == event_buffers.end()) {
            event_buffers[id] = std::make_unique<EventContainer<T>>();
        }

        auto* container = static_cast<EventContainer<T>*>(event_buffers[id].get());
        container->events.push_back(event);
    }

    /**
     * @brief Retrieves all buffered events of a specific type (Used by the Dispatcher/Tests).
     */
    template<typename T>
    const std::vector<T>& getEvents() {
        EventID id = EventTypeRegistry::getID<T>();

        if (event_buffers.find(id) == event_buffers.end()) {
            event_buffers[id] = std::make_unique<EventContainer<T>>();
        }

        auto* container = static_cast<EventContainer<T>*>(event_buffers[id].get());
        return container->events;
    }

    /**
     * @brief Clears all event buffers. Usually called at the end of the tick.
     */
    void clearAll() {
        for (auto& [id, container] : event_buffers) {
            container->clear();
        }
    }
};