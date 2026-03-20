#pragma once

#include <functional>

/**
 * @file event_filter.hpp
 * @brief Middleware that wraps event callbacks with a conditional predicate gate.
 */

class EventFilter {
public:
    /**
     * @brief Wraps a standard callback with a true/false condition.
     * @tparam T The specific event struct.
     * @param predicate A lambda returning true if the event should be processed.
     * @param callback The actual logic to execute if the predicate passes.
     * @return A composite std::function ready to be passed directly to the SubscriberRegistry.
     */
    template<typename T>
    static std::function<void(const T&)> create(
        std::function<bool(const T&)> predicate,
        std::function<void(const T&)> callback
    ) {
        return [predicate, callback](const T& event) {
            if (predicate(event)) {
                callback(event);
            }
        };
    }
};