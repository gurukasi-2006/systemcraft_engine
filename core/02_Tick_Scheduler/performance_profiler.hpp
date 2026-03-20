#pragma once

#include <chrono>
#include <string>
#include <map>
#include <iostream>
#include <iomanip>

/**
 * @file performance_profiler.hpp
 * @brief Records execution times of systems and calculates rolling averages.
 */

class PerformanceProfiler {
private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    /**
     * @brief Holds the analytics data for a single tracked system.
     */
    struct ProfileData {
        float rolling_average_ms = 0.0f;
        uint64_t call_count = 0;
    };

    /**
     * @brief A map linking the string name of a system to its analytics.
     */
    std::map<std::string, ProfileData> system_profiles;

    /**
     * @brief The starting timestamp for the currently running system.
     */
    TimePoint current_start;

public:
    /**
     * @brief Starts the high-precision microsecond stopwatch.
     * * MUST be called exactly before a system's update() function.
     */
    void beginTimer() {
        current_start = Clock::now();
    }

    /**
     * @brief Stops the stopwatch and logs the data to the specified system.
     * * MUST be called exactly after a system's update() function finishes.
     * @param system_name The human-readable name of the system (e.g., "PhysicsSystem").
     */
    void endTimer(const std::string& system_name) {
        auto end_time = Clock::now();


        std::chrono::duration<float, std::milli> elapsed = end_time - current_start;
        float ms = elapsed.count();

        auto& data = system_profiles[system_name];

        if (data.call_count == 0) {

            data.rolling_average_ms = ms;
        } else {
            data.rolling_average_ms = (data.rolling_average_ms * 0.9f) + (ms * 0.1f);
        }
        data.call_count++;
    }

    /**
     * @brief Retrieves the smoothed average time a system takes to run.
     * @return Execution time in milliseconds.
     */
    float getAverage(const std::string& system_name) {
        return system_profiles[system_name].rolling_average_ms;
    }

    /**
     * @brief Prints a formatted performance report to the terminal.
     */
    void printReport() const {
        std::cout << "\n=== Engine Performance Profiler Report ===\n";
        for (const auto& [name, data] : system_profiles) {
            std::cout << std::left << std::setw(20) << name
                      << " | Avg: " << data.rolling_average_ms << " ms "
                      << " | Total Calls: " << data.call_count << "\n";
        }
        std::cout << "==========================================\n";
    }
};