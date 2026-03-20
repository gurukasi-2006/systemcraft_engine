#pragma once

#include <memory>
#include <functional>

/**
 * @file unsubscribe_guard.hpp
 * @brief RAII memory shield that automatically invalidates callbacks when its owning system is destroyed.
 */

class UnsubscribeGuard {
private:
    std::shared_ptr<bool> lifeline;

public:
    UnsubscribeGuard() : lifeline(std::make_shared<bool>(true)) {}

    /**
     * @brief Wraps a callback in a protective weak_ptr check.
     * @tparam T The specific event struct.
     * @param callback The dangerous logic that touches the system's memory.
     * @return A safe lambda ready for the SubscriberRegistry.
     */
    template<typename T>
    std::function<void(const T&)> bind(std::function<void(const T&)> callback) {

        std::weak_ptr<bool> weak_lifeline = lifeline;

        return [weak_lifeline, callback](const T& event) {

            if (weak_lifeline.lock()) {
                callback(event);
            } else {

            }
        };
    }
};