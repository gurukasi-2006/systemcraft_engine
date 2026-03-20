#pragma once

#include "subscriber_registry.hpp"

/**
 * @file immediate_dispatcher.hpp
 * @brief Synchronously routes events directly to subscribers the exact moment they are fired.
 */

class ImmediateDispatcher {
private:
    /**
     * @brief A reference to the master phonebook so we know who to call.
     */
    SubscriberRegistry& registry;

public:
    /**
     * @brief Constructs the dispatcher and links it to your registry.
     * @param reg The active SubscriberRegistry.
     */
    ImmediateDispatcher(SubscriberRegistry& reg) : registry(reg) {}

    /**
     * @brief Fires an event and immediately blocks the current thread until all subscribers finish executing.
     * @tparam T The specific event struct.
     * @param event The data payload.
     */
    template<typename T>
    void dispatch(const T& event) {
        const auto& callbacks = registry.getCallbacks<T>();

        for (const auto& callback : callbacks) {
            callback(event);
        }
    }
};