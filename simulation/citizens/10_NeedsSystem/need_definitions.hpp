#pragma once

#include <cstdint>
#include <array>
#include <algorithm>

/**
 * @file need_definitions.hpp
 * @brief Subsystem 115: Defines the comprehensive 13-tier needs system for citizens.
 */

namespace Population {

    /**
     * @enum NeedType
     * @brief The 13 core psychological and physical needs of a Systemcraft citizen.
     */
    enum class NeedType : uint8_t {
        // --- Tier 1: Survival ---
        Food = 0,
        Water,
        Shelter,
        Healthcare,

        // --- Tier 2: Stability ---
        Safety,
        Employment,
        Transport,

        // --- Tier 3: Societal ---
        Education,
        Leisure,

        // --- Tier 4: Modernity & Self-Actualization (The Systemcraft Unique Needs) ---
        Community,     ///< Satisfaction driven by density of local SocialNetwork links
        Environment,   ///< Satisfaction driven by lack of local pollution and proximity to nature
        ConsumerGoods, ///< Demand for manufactured products (furniture, appliances, clothes)
        Connectivity,  ///< Demand for telecom, internet, and media access

        COUNT          ///< Automatically resolves to 13
    };

    /**
     * @struct NeedsComponent
     * @brief ECS Component holding the satisfaction array (0.0 to 100.0) for all needs.
     * @details Uses a contiguous std::array for zero-allocation, cache-friendly querying.
     */
    struct NeedsComponent {
        std::array<float, static_cast<size_t>(NeedType::COUNT)> satisfaction_levels;

        /**
         * @brief Initializes all needs to a neutral baseline of 50.0.
         */
        NeedsComponent() {
            satisfaction_levels.fill(50.0f);
        }

        /**
         * @brief Retrieves the current satisfaction of a specific need.
         */
        inline float get_need(NeedType type) const {
            return satisfaction_levels[static_cast<size_t>(type)];
        }

        /**
         * @brief Hard-sets a need, automatically clamping to [0.0, 100.0].
         */
        inline void set_need(NeedType type, float value) {
            satisfaction_levels[static_cast<size_t>(type)] = std::clamp(value, 0.0f, 100.0f);
        }

        /**
         * @brief Safely adds or subtracts from a need, automatically clamping bounds.
         * @param type The specific need to modify.
         * @param delta Positive to satisfy, Negative to degrade.
         */
        inline void add_to_need(NeedType type, float delta) {
            float current = satisfaction_levels[static_cast<size_t>(type)];
            satisfaction_levels[static_cast<size_t>(type)] = std::clamp(current + delta, 0.0f, 100.0f);
        }
    };
}