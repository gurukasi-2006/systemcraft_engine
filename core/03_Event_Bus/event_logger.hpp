#pragma once

#include <fstream>
#include <string>
#include <functional>
#include "subscriber_registry.hpp"
#include "../02_Tick_Scheduler/tick_counter.hpp"

/**
 * @file event_logger.hpp
 * @brief Intercepts registered events and writes them to a persistent log file with timestamps.
 */

class EventLogger {
private:
    std::ofstream log_file;
    SubscriberRegistry& registry;
    const TickCounter& clock;

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
    }

    /**
     * @brief Tells the logger to intercept a specific event type and how to write it as text.
     * @tparam T The specific event struct.
     * @param event_name A human-readable name for the log file.
     * @param serializer A lambda that converts the event's internal data into a string.
     */
    template<typename T>
    void trackEvent(const std::string& event_name, std::function<std::string(const T&)> serializer) {
        registry.subscribe<T>([this, event_name, serializer](const T& event) {
            if (log_file.is_open()) {
                log_file << "[Tick: " << clock.get() << "] [" << event_name << "] "
                         << serializer(event) << "\n";

                log_file.flush();
            }
        });
    }
};