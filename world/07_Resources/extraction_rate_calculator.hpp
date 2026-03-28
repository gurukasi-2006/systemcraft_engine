#pragma once

#include <cstdint>
#include "../../core/04_Types/fixed_point.hpp"
#include "resource_deposit_component.hpp"

/**
 * @file extraction_rate_calculator.hpp
 * @brief Computes resource extraction yields per tick based on geological and economic factors.
 */

namespace Resources {

    class ExtractionRateCalculator {
    public:
        /**
         * @brief Calculates how much of a resource is extracted in a single engine tick.
         * @param deposit The physical resource deposit being mined (read-only).
         * @param facility_level The technological level of the mine (e.g., 1, 2, 3).
         * @param worker_count The number of citizens currently operating the facility.
         * @return The exact amount of resources extracted this tick.
         */
        static Fixed32 calculate_yield(
            const ResourceDepositComponent& deposit,
            int32_t facility_level,
            int32_t worker_count)
        {
            // If the mine is abandoned, unpowered, or the deposit is dry, output is zero.
            if (worker_count <= 0 || facility_level <= 0 || deposit.total_reserve_quantity <= Fixed32(0.0f)) {
                return Fixed32(0.0f);
            }

            // Base extraction per worker per tick
            Fixed32 base_rate_per_worker(0.5f);

            // Facility level multiplier: Level 1 = 1.0x, Level 2 = 1.5x, Level 3 = 2.0x
            Fixed32 facility_multiplier(1.0f + (static_cast<float>(facility_level) - 1.0f) * 0.5f);

            // Gross Yield = Workers * Base Rate * Facility Multiplier
            Fixed32 gross_yield = Fixed32(static_cast<float>(worker_count)) * base_rate_per_worker * facility_multiplier;

            // Net Yield = Gross Yield / Geological Difficulty
            Fixed32 net_yield = gross_yield / deposit.extraction_difficulty;

            // Physical Constraint: You cannot mine more than what actually exists in the ground.
            if (net_yield > deposit.total_reserve_quantity) {
                return deposit.total_reserve_quantity;
            }

            return net_yield;
        }
    };
}