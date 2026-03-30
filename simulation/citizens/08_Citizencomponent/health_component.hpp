#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>

// Project Specific Includes
// Path assumes: root/simulation/citizens/08_Citizencomponent/health_component.hpp
// pointing to root/data/tables/disease_tables.hpp
#include "../data/tables/disease_tables.hpp"

/**
 * @file health_component.hpp
 * @brief Subsystem 92: Tracks physical and mental health, processing environmental decay and healthcare recovery.
 */

namespace Population {

    /**
     * @struct HealthComponent
     * @brief ECS Component holding vital statistics and actively processing health equations.
     */
    struct HealthComponent {
        float physical_health{100.0f};  // 0.0 to 100.0
        float mental_health{100.0f};    // 0.0 to 100.0
        bool disability_flag{false};

        // Stores indices that map directly to DISEASE_TABLE in disease_tables.hpp
        std::vector<uint32_t> active_diseases;

        /**
         * @brief Processes one simulation tick of health decay, disease drain, and recovery.
         * @param base_decay Intrinsic baseline health loss (e.g., aging factor).
         * @param need_penalty Modifier based on starvation/unmet needs.
         * @param food_sat Current food satisfaction level (0.0 to 100.0).
         * @param healthcare_gap Deficit in regional healthcare coverage.
         * @param safety_penalty Environmental hazards (e.g., crime, pollution).
         * @param healthcare_quality The quality of access the citizen currently has.
         */
        inline void update(float base_decay, float need_penalty, float food_sat,
                           float healthcare_gap, float safety_penalty, float healthcare_quality)
        {
            // 1. Calculate environmental decay from unmet needs
            float decay = base_decay
                        + (need_penalty * (1.0f - (food_sat / 100.0f)) * 0.02f)
                        + (healthcare_gap * 0.005f)
                        + (safety_penalty * 0.003f);

            // 2. Calculate active disease drains
            float phys_disease_drain = 0.0f;
            float mental_disease_drain = 0.0f;

            for (uint32_t disease_idx : active_diseases) {
                if (disease_idx < DISEASE_TABLE.size()) {
                    phys_disease_drain += DISEASE_TABLE[disease_idx].phys_drain;
                    mental_disease_drain += DISEASE_TABLE[disease_idx].mental_drain;
                }
            }

            // 3. Calculate recovery from healthcare access
            float recovery = healthcare_quality * 0.01f;

            // Cap maximum possible medical recovery per tick
            if (recovery > 0.5f) {
                recovery = 0.5f;
            }

            // Clearing all diseases grants a massive immune/morale boost
            if (active_diseases.empty()) {
                recovery *= 2.0f;
            }

            // 4. Apply net changes (Recovery fights against Decay and Disease)
            physical_health = physical_health + recovery - decay - phys_disease_drain;
            mental_health = mental_health + recovery - decay - mental_disease_drain;

            // 5. Enforce biological bounds
            physical_health = std::clamp(physical_health, 0.0f, 100.0f);
            mental_health = std::clamp(mental_health, 0.0f, 100.0f);
        }
    };
}