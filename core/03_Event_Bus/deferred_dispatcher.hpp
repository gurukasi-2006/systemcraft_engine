#pragma once

#include "event_publisher.hpp"
#include "subscriber_registry.hpp"

/**
 * @file deferred_dispatcher.hpp
 * @brief Flushes buffered events and routes them to subscribers at a safe, designated time.
 */

class DeferredDispatcher {
private:
    EventPublisher& publisher;
    SubscriberRegistry& registry;

public:
    /**
     * @brief Constructs the dispatcher and links it to your global messaging systems.
     */
    DeferredDispatcher(EventPublisher& pub, SubscriberRegistry& reg)
        : publisher(pub), registry(reg) {}

    /**
     * @brief Flushes all buffered events of a specific type and executes their callbacks.
     * * Usually called at the end of the game loop to prevent logic cascades.
     * @tparam T The specific event struct to process.
     */
    template<typename T>
    void flush() {
        const auto& pending_events = publisher.getEvents<T>();

        const auto& callbacks = registry.getCallbacks<T>();

        for (const auto& event : pending_events) {
            for (const auto& callback : callbacks) {
                callback(event);
            }
        }
    }

    /**
     * @brief Clears the master publisher buffer.
     * * MUST be called after you have flushed all your specific event types!
     */
    void clearBuffers() {
        publisher.clearAll();
    }
};