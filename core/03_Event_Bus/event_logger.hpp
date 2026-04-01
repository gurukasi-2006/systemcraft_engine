#pragma once

#include <fstream>
#include <string>
#include <functional>
#include <vector>

#include "subscriber_registry.hpp"
#include "unsubscribe_guard.hpp"
#include "../02_Tick_Scheduler/tick_counter.hpp"

/**
 * @file event_logger.hpp
 * @brief Intercepts registered events and writes them to a persistent log file with timestamps.
 * @details Automatically cleans up event subscriptions upon destruction to prevent dangling pointers.
 */

class EventLogger {
private:
    std::ofstream log_file;
    SubscriberRegistry& registry;
    const TickCounter& clock;

    // RAII Guards to automatically unsubscribe when the logger is destroyed
    std::vector<UnsubscribeGuard> subscription_guards;

public:
    /**
     * @brief Opens the log file and links the logger to the engine's subsystems.
     */
    EventLogger(const std::string& filepath, SubscriberRegistry& reg, const TickCounter& clk)
        : registry(reg), clock(clk) {

        log_file.open(filepath, std::ios::out | std::ios::trunc);
        if (log_file.is_open()) {
            log_file << "=== SYSTEMCRAFT ENGINE LOG INITIALIZED ===\n";
        }
    }

    ~EventLogger() {
        if (log_file.is_open()) {
            log_file << "=== LOG TERMINATED ===\n";
            log_file.close();
        }
        // subscription_guards vector is automatically destroyed here,
        // triggering the destructors of UnsubscribeGuard to safely remove callbacks from the registry.
    }

    /**
     * @brief Tells the logger to intercept a specific event type and how to write it as text.
     * @tparam T The specific event struct.
     * @param event_name A human-readable name for the log file.
     * @param serializer A lambda that converts the event's internal data into a string.
     */
    template<typename T>
    void trackEvent(const std::string& event_name, std::function<std::string(const T&)> serializer) {

        // 1. Create a new RAII guard and store it in our vector so it lives as long as the Logger
        subscription_guards.emplace_back();

        // 2. Wrap our dangerous lambda with the guard's weak_ptr safety check
        auto safe_callback = subscription_guards.back().bind<T>([this, event_name, serializer](const T& event) {
            if (log_file.is_open()) {
                log_file << "[" << clock.get() << "] "
                         << event_name << ": "
                         << serializer(event) << "\n";
            }
        });

        // 3. Register the safe, wrapped callback to the Event Bus
        registry.subscribe<T>(safe_callback);
    }
};
