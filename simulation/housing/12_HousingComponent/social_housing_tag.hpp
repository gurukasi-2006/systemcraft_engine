#pragma once

#include <cstdint>
#include <algorithm>

/**
 * @file social_housing_tag.hpp
 * @brief Subsystem 150: Zero-byte ECS tag marking publicly owned, rent-controlled buildings.
 */

namespace Housing {

    /**
     * @struct SocialHousingTag
     * @brief A zero-size tag component. Presence exempts the building from market-rate rent calculations.
     */
    struct SocialHousingTag {};

    /**
     * @class SocialHousingPolicy
     * @brief Centralizes the macroeconomic logic for government-subsidized rent calculations.
     */
    class SocialHousingPolicy {
    public:
        /**
         * @brief Calculates the mandated rent for social housing based on macroeconomic median income.
         * @param median_income The current median income of the city's population.
         * @return The capped rent price (strictly 25% of median income).
         */
        static float calculate_rent(float median_income) {
            return median_income * 0.25f;
        }

        /**
         * @brief Calculates the ongoing subsidy cost to the government treasury for maintaining this unit.
         * @param market_rent What the building would fetch on the open capitalist market.
         * @param median_income The current median income used for social rent.
         * @return The financial differential the government must pay to cover the gap.
         */
        static float calculate_government_subsidy(float market_rent, float median_income) {
            float social_rent = calculate_rent(median_income);

            // If the market is completely crashed and cheaper than social rent, no subsidy is needed
            if (social_rent >= market_rent) {
                return 0.0f;
            }
            return market_rent - social_rent;
        }
    };
}