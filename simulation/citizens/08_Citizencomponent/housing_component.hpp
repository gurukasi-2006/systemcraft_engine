#pragma once

#include <cstdint>
#include <algorithm>

#include "../../../core/04_Types/entity_id.hpp"
#include "../../../core/04_Types/fixed_point.hpp"

/**
 * @file housing_component.hpp
 * @brief Subsystem 96: Tracks citizen housing status, shelter satisfaction, and eviction logic.
 */

namespace Population {

    /**
     * @enum OccupancyStatus
     * @brief Determines the physical living situation of the citizen.
     */
    enum class OccupancyStatus : uint8_t {
        Homeless = 0,
        Housed = 1
    };

    /**
     * @struct HousingComponent
     * @brief ECS Component linking a citizen to physical real estate.
     */
    struct HousingComponent {
        EntityID housing_entity{0};                        ///< The ECS ID of the specific residential building
        Fixed32 monthly_rent{0.0f};                        ///< The contract rent or mortgage rate
        OccupancyStatus occupancy_status{OccupancyStatus::Homeless};
        float housing_quality_score{0.0f};                 ///< Ranges from 0.0 (Slum) to 100.0 (Luxury)

        uint8_t months_in_default{0};                      ///< Tracks consecutive missed payments

        /**
         * @brief Calculates the effective shelter satisfaction for the Happiness Aggregator.
         * @return Quality score if housed, absolute 0.0 if homeless.
         */
        inline float get_shelter_satisfaction() const {
            float occupancy_factor = (occupancy_status == OccupancyStatus::Housed) ? 1.0f : 0.0f;
            return housing_quality_score * occupancy_factor;
        }

        /**
         * @brief Processes a monthly rent cycle, handling the 3-month eviction rule.
         * @param payment_successful True if the citizen had enough savings/debt capacity to pay.
         * @return True if the citizen was evicted this month as a result of the default.
         */
        inline bool process_rent_cycle(bool payment_successful) {
            if (occupancy_status == OccupancyStatus::Homeless) {
                return false; // Already homeless, cannot be evicted again
            }

            if (payment_successful) {
                months_in_default = 0; // A successful payment completely resets the delinquency tracker
                return false;
            } else {
                months_in_default++;

                // The 3 Consecutive Months Eviction Rule
                if (months_in_default >= 3) {
                    make_homeless();
                    return true; // Flags that an eviction event just occurred
                }
                return false;
            }
        }

        /**
         * @brief Instantly severs the citizen from their home (e.g., building destroyed, evicted, migrated).
         */
        inline void make_homeless() {
            housing_entity = EntityID{0};
            occupancy_status = OccupancyStatus::Homeless;
            housing_quality_score = 0.0f;
            monthly_rent = Fixed32(0.0f);
            months_in_default = 0;
        }

        /**
         * @brief Moves the citizen into a new residential building.
         */
        inline void move_in(EntityID home, Fixed32 rent, float quality) {
            housing_entity = home;
            monthly_rent = rent;
            housing_quality_score = std::clamp(quality, 0.0f, 100.0f);
            occupancy_status = OccupancyStatus::Housed;
            months_in_default = 0;
        }
    };
}