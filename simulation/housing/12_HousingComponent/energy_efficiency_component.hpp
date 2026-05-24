#pragma once

#include <cstdint>
#include <algorithm>

/**
 * @file energy_efficiency_component.hpp
 * @brief Subsystem 151: Tracks building insulation, energy consumption, and green policy incentives.
 */

namespace Housing {

    /**
     * @enum ConstructionStandard
     * @brief Represents the technological era and environmental compliance of the building.
     */
    enum class ConstructionStandard : uint8_t {
        Pre1980 = 0,
        Modern,
        GreenCertified
    };

    /**
     * @struct EnergyEfficiencyComponent
     * @brief Handles utility operating costs and environmental tax breaks for real estate.
     */
    struct EnergyEfficiencyComponent {
        float insulation_rating{0.5f}; // Range: 0.0 to 1.0
        float annual_energy_kwh_per_m2{150.0f};
        ConstructionStandard standard{ConstructionStandard::Modern};

        /**
         * @brief Calculates the monthly utility bill based on efficiency and insulation.
         * @param floor_area_m2 Total square meterage of the building.
         * @param electricity_price_per_kwh The city's current macroeconomic grid price.
         * @return The monthly energy cost in dollars.
         */
        float calculate_monthly_energy_cost(float floor_area_m2, float electricity_price_per_kwh) const {
            float base_monthly_consumption = (floor_area_m2 * annual_energy_kwh_per_m2) / 12.0f;

            // Perfect insulation (1.0) reduces energy cost by a maximum of 40%
            float insulation_modifier = 1.0f - (insulation_rating * 0.4f);

            return base_monthly_consumption * electricity_price_per_kwh * insulation_modifier;
        }

        /**
         * @brief Returns the rent premium multiplier for green certified buildings.
         * @details Tenants are willing to pay higher rent because their utility bills are lower.
         */
        float get_rent_premium_multiplier() const {
            if (standard == ConstructionStandard::GreenCertified) {
                return 1.08f; // 8% premium
            }
            return 1.0f; // No premium
        }

        /**
         * @brief Calculates the one-time government tax credit for building green.
         * @param construction_cost The total CAPEX required to build the structure.
         */
        float get_tax_credit(float construction_cost) const {
            if (standard == ConstructionStandard::GreenCertified) {
                return construction_cost * 0.12f; // 12% tax credit
            }
            return 0.0f;
        }
    };
}