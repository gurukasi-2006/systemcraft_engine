#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>

#include "../../../world/06_Worldgen/seed_manager.hpp"
#include "employment_component.hpp" // To access JobType for base schedules

// Fallback for mathematical pi if not defined by standard headers
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @file daily_schedule_component.hpp
 * @brief Subsystem 100: Defines the citizen's daily routine to generate organic transport demand.
 */

namespace Population {

    /**
     * @struct DailyScheduleComponent
     * @brief ECS Component holding the specific ticks within a 24-tick day when the citizen transitions state.
     */
    struct DailyScheduleComponent {
        uint8_t wake_tick{0};
        uint8_t work_start_tick{0};
        uint8_t work_end_tick{0};
        uint8_t sleep_tick{0};

        /**
         * @brief Generates a realistic daily routine based on the citizen's job and commute.
         * @param rng The deterministic random number generator.
         * @param job_type The category of their employment (determines the base shift).
         * @param contracted_hours The length of their workday (e.g., 8 hours).
         * @param commute_ticks How long it takes them to travel to work.
         */
        inline void generate_schedule(SeedManager& rng, JobType job_type, uint8_t contracted_hours, uint8_t commute_ticks) {

            // 1. Determine Base Start Time based on Job Type
            float base_start = 8.0f; // Default 8:00 AM

            switch (job_type) {
                case JobType::ManualLabor:
                    base_start = 6.0f; // Early shift (Construction, Factories)
                    break;
                case JobType::Service:
                    base_start = 10.0f; // Late morning shift (Retail, Restaurants)
                    break;
                case JobType::Professional:
                case JobType::Management:
                    base_start = 9.0f; // Standard office hours
                    break;
                case JobType::None:
                    // Unemployed/Retired citizens don't have a strict work start,
                    // but we generate a "busy" period for shopping/leisure traffic.
                    base_start = 11.0f;
                    contracted_hours = 4; // Shorter active period
                    break;
            }

            // 2. Apply Gaussian Jitter: Normal(0, sigma=1.5) using Box-Muller Transform
            float u1 = rng.random_fixed(0.0001f, 1.0f).toFloat();
            float u2 = rng.random_fixed(0.0f, 1.0f).toFloat();
            float z0 = std::sqrt(-2.0f * std::log(u1)) * std::cos(2.0f * static_cast<float>(M_PI) * u2);

            float jitter = z0 * 1.5f;

            // 3. Calculate exact ticks (Ensuring they wrap neatly within a 24-tick/hour cycle)
            int32_t start = static_cast<int32_t>(std::round(base_start + jitter));

            // Clamp to prevent extreme outliers from wrapping into the next day weirdly
            start = std::clamp(start, 4, 16);

            int32_t end = start + contracted_hours;
            int32_t wake = start - commute_ticks - 1; // Wake up with 1 tick to spare before leaving
            int32_t sleep = end + 4; // Go to sleep 4 ticks/hours after getting home

            // 4. Wrap values into the 24-tick cycle (modulo 24)
            work_start_tick = static_cast<uint8_t>((start % 24 + 24) % 24);
            work_end_tick = static_cast<uint8_t>((end % 24 + 24) % 24);
            wake_tick = static_cast<uint8_t>((wake % 24 + 24) % 24);
            sleep_tick = static_cast<uint8_t>((sleep % 24 + 24) % 24);
        }

        /**
         * @brief Helper for the Transport System to check if the citizen should be traveling to work right now.
         * @param current_hour_tick The current time of day (0 to 23).
         */
        inline bool is_commuting_to_work(uint8_t current_hour_tick) const {
            // They leave home exactly when they wake up (since wake = start - commute - 1)
            // and travel until work starts.
            if (wake_tick < work_start_tick) {
                return current_hour_tick >= wake_tick && current_hour_tick < work_start_tick;
            } else {
                // Handles the midnight wrap-around (e.g., wake at 23, start at 1)
                return current_hour_tick >= wake_tick || current_hour_tick < work_start_tick;
            }
        }
    };
}