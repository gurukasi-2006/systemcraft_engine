#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>

#include "../../../core/04_Types/time_constants.hpp"
#include "../../../world/06_Worldgen/seed_manager.hpp"


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @file age_lifecycle_component.hpp
 * @brief Tracks the physical aging and lifecycle stages of citizens.
 */

namespace Population {

    /**
     * @enum LifeStage
     * @brief Represents the current biological and social phase of the citizen.
     */
    enum class LifeStage : uint8_t {
        Child = 0, ///< Age < 18: Cannot work, consumes less food.
        Adult = 1, ///< Age 18-64: Core workforce.
        Elder = 2  ///< Age >= 65: Retired, high mortality risk.
    };

    /**
     * @struct AgeLifecycleComponent
     * @brief An actively updated ECS component holding the citizen's mortality and age data.
     */
    struct AgeLifecycleComponent {
        uint64_t current_age_ticks{0};
        LifeStage stage{LifeStage::Child};
        uint32_t expected_lifespan_years{75};

        /**
         * @brief Updates the age and life stage based on absolute simulation time.
         * @details Designed to be called continuously by the Citizen System.
         * @param current_tick The engine's master time.
         * @param birth_tick The immutable tick the citizen was born (from IdentityComponent).
         * @param ticks_per_year The global conversion rate (passed explicitly to avoid hardcoded dependencies).
         */
        inline void update(uint64_t current_tick, uint64_t birth_tick, uint64_t ticks_per_year) {
            if (current_tick > birth_tick) {
                current_age_ticks = current_tick - birth_tick;
            } else {
                current_age_ticks = 0;
            }

            // Convert to human-readable years to evaluate biological thresholds
            uint64_t age_years = current_age_ticks / ticks_per_year;

            // Update Life Stage Enum
            if (age_years < 18) {
                stage = LifeStage::Child;
            } else if (age_years < 65) {
                stage = LifeStage::Adult;
            } else {
                stage = LifeStage::Elder;
            }
        }
    };

    /**
     * @class LifespanGenerator
     * @brief Computes genetically/environmentally deterministic lifespans.
     */
    class LifespanGenerator {
    public:
        /**
         * @brief Generates a bell-curved lifespan using the Box-Muller transform.
         * @param rng The deterministic SeedManager.
         * @return Expected lifespan in years, strictly clamped to [45, 100].
         */
        static uint32_t generate_expected_lifespan(SeedManager& rng) {
            // Box-Muller Transform requires two uniform random variables (0, 1]
            // We use 0.0001f for u1 to strictly prevent std::log(0) which causes NaN explosions
            float u1 = rng.random_fixed(0.0001f, 1.0f).toFloat();
            float u2 = rng.random_fixed(0.0f, 1.0f).toFloat();

            // Generate Standard Normal (Z-score)
            float z0 = std::sqrt(-2.0f * std::log(u1)) * std::cos(2.0f * static_cast<float>(M_PI) * u2);

            // Apply parameters: Normal(mu=75, sigma=10)
            float lifespan = 75.0f + (z0 * 10.0f);

            // Enforce the biological boundaries of the simulation
            return std::clamp(static_cast<uint32_t>(std::round(lifespan)), 45u, 100u);
        }
    };
}