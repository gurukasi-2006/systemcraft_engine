#pragma once

#include <chrono>
#include <string>
#include <map>
#include <iostream>
#include <iomanip>

#include "../04_Types/engine_assert.hpp"

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

    /**
     * @brief Guard flag to prevent mismatched timer calls.
     */
    bool is_active_ = false;

public:
    /**
     * @brief Starts the high-precision microsecond stopwatch.
     * * MUST be called exactly before a system's update() function.
     */
    void beginTimer() {
        ENGINE_ASSERT(!is_active_, "PerformanceProfiler: beginTimer() called twice without endTimer()!");
        current_start = Clock::now();
        is_active_ = true;
    }

    /**
     * @brief Stops the stopwatch and logs the data to the specified system.
     * * MUST be called exactly after a system's update() function finishes.
     * @param system_name The human-readable name of the system (e.g., "PhysicsSystem").
     */
    void endTimer(const std::string& system_name) {
        ENGINE_ASSERT(is_active_, "PerformanceProfiler: endTimer() called without beginTimer()!");
        auto end_time = Clock::now();

        // Reset the guard flag immediately
        is_active_ = false;

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
            std::cout << std::left << std::setw(30) << name
                      << ": " << std::fixed << std::setprecision(4)
                      << data.rolling_average_ms << " ms (avg over "
                      << data.call_count << " ticks)\n";
        }
        std::cout << "==========================================\n";
    }
};
