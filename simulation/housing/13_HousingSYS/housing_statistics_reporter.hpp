#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"

// Housing Components
#include "../12_HousingComponent/rent_valuation_component.hpp"
#include "../12_HousingComponent/occupancy_component.hpp"
#include "../12_HousingComponent/social_housing_tag.hpp"
#include "../12_HousingComponent/building_identity_component.hpp"

/**
 * @file housing_statistics_reporter.hpp
 * @brief Subsystem 165: Aggregates macroeconomic housing data and detects real estate bubbles.
 */

namespace Housing {

    // --- Official Statistics Events ---

    struct HousingReportEvent {
        uint32_t region_id;
        float vacancy_rate;
        float avg_rent;
        float homeownership_rate;
        float overcrowding_pct;
        float median_rent_ratio; // Affordability Index
        float price_to_rent_ratio; // P/E Ratio
    };

    struct HousingBubbleWarningEvent {
        uint32_t region_id;
        float price_to_rent_ratio;
    };

    /**
     * @class HousingStatisticsReporter
     * @brief Computes monthly metrics across all buildings for the Stats Bureau.
     */
    class HousingStatisticsReporter {
    public:
        /**
         * @brief Sweeps the city to aggregate housing metrics.
         * @param region_id The geographic zone being surveyed.
         * @param median_income The current city-wide median income for affordability calculations.
         */
        void generate_monthly_report(ECSWorld& world, EventPublisher& publisher, uint32_t region_id, float median_income) {

            // Counters
            uint32_t total_units = 0;
            uint32_t vacant_units = 0;
            uint32_t overcrowded_units = 0;
            uint32_t social_units = 0;

            // Accumulators
            float sum_rent_occupied_private = 0.0f;
            uint32_t occupied_private_count = 0;

            float sum_price_to_rent = 0.0f;
            uint32_t private_valuation_count = 0;

            // Query all housing
            auto view = world.registry.view<OccupancyComponent, RentValuationComponent>();

            for (auto raw : view) {
                const auto& occ = view.get<OccupancyComponent>(raw);
                const auto& rent = view.get<RentValuationComponent>(raw);

                total_units++;

                // Vacancy Check
                bool is_vacant = occ.occupant_list.empty();
                if (is_vacant) vacant_units++;

                // Overcrowding Check
                if (occ.occupant_list.size() > occ.max_capacity) overcrowded_units++;

                // Social Housing (Homeownership Proxy based on design doc)
                bool is_social = world.registry.all_of<SocialHousingTag>(raw);
                if (is_social) social_units++;

                // Private Market Accumulators
                if (!is_social) {
                    // Avg rent only counts occupied private units
                    if (!is_vacant) {
                        sum_rent_occupied_private += rent.monthly_rent;
                        occupied_private_count++;
                    }

                    // P/E Ratio: Market Valuation / Annual Rent
                    float annual_rent = rent.monthly_rent * 12.0f;
                    if (annual_rent > 0.0f) {
                        float pe_ratio = rent.market_valuation / annual_rent;
                        sum_price_to_rent += pe_ratio;
                        private_valuation_count++;
                    }
                }
            }

            // Guard against division by zero in ghost towns
            if (total_units == 0) return;

            // --- Compute Final Metrics ---

            float vacancy_rate = static_cast<float>(vacant_units) / static_cast<float>(total_units);
            float homeownership_rate = static_cast<float>(social_units) / static_cast<float>(total_units);
            float overcrowding_pct = static_cast<float>(overcrowded_units) / static_cast<float>(total_units);

            float avg_rent = 0.0f;
            if (occupied_private_count > 0) {
                avg_rent = sum_rent_occupied_private / static_cast<float>(occupied_private_count);
            }

            float median_rent_ratio = 0.0f;
            if (median_income > 0.0f) {
                median_rent_ratio = avg_rent / median_income;
            }

            float avg_price_to_rent = 0.0f;
            if (private_valuation_count > 0) {
                avg_price_to_rent = sum_price_to_rent / static_cast<float>(private_valuation_count);
            }

            // --- Dispatch Reports ---

            publisher.publish(HousingReportEvent{
                region_id,
                vacancy_rate,
                avg_rent,
                homeownership_rate,
                overcrowding_pct,
                median_rent_ratio,
                avg_price_to_rent
            });

            // 25.0 P/E triggers Bubble Warning
            if (avg_price_to_rent > 25.0f) {
                publisher.publish(HousingBubbleWarningEvent{region_id, avg_price_to_rent});
            }
        }
    };
}