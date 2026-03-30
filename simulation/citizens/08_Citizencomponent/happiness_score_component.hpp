#pragma once

#include <algorithm>

/**
 * @file happiness_score_component.hpp
 * @brief Subsystem 101: Calculates citizen happiness using weighted needs and exponential smoothing.
 */

namespace Population {

    /**
     * @struct HappinessScoreComponent
     * @brief ECS Component holding the citizen's composite mood.
     */
    struct HappinessScoreComponent {
        // Starts at a neutral 50.0 to prevent massive initial swings
        float current_happiness{50.0f};

        /**
         * @brief Updates the citizen's happiness score, incorporating policy lag via memory decay.
         * @param food_sat Satisfaction from diet/caloric intake (0.0 - 100.0).
         * @param shelter_sat Satisfaction from housing quality and stability (0.0 - 100.0).
         * @param safety_sat Satisfaction from low crime/pollution (0.0 - 100.0).
         * @param healthcare_sat Satisfaction from medical access and health (0.0 - 100.0).
         * @param employment_sat Satisfaction from job security/wages (0.0 - 100.0).
         * @param transport_sat Satisfaction from commute times and transit access (0.0 - 100.0).
         * @param leisure_sat Satisfaction from entertainment and parks (0.0 - 100.0).
         * @param education_sat Satisfaction from schooling access (0.0 - 100.0).
         * @param water_sat Satisfaction from utility reliability (0.0 - 100.0).
         */
        inline void update(float food_sat, float shelter_sat, float safety_sat,
                           float healthcare_sat, float employment_sat, float transport_sat,
                           float leisure_sat, float education_sat, float water_sat)
        {
            // 1. Calculate the instantaneous raw happiness based on hardcoded sociological weights
            float raw_happiness =
                  (0.25f * food_sat)
                + (0.15f * shelter_sat)
                + (0.12f * safety_sat)
                + (0.12f * healthcare_sat)
                + (0.10f * employment_sat)
                + (0.08f * transport_sat)
                + (0.08f * leisure_sat)
                + (0.06f * education_sat)
                + (0.04f * water_sat);

            // Clamp raw happiness just in case any inputs exceeded boundaries
            raw_happiness = std::clamp(raw_happiness, 0.0f, 100.0f);

            // 2. Apply Exponential Smoothing (Memory Decay)
            // alpha = 0.08 means the new conditions account for 8% of the mood shift this tick,
            // while 92% of their mood is anchored in their recent past memory.
            current_happiness = (0.08f * raw_happiness) + (0.92f * current_happiness);

            // Enforce final bounds
            current_happiness = std::clamp(current_happiness, 0.0f, 100.0f);
        }
    };
}