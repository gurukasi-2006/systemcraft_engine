#pragma once

#include <cstdint>
#include <algorithm>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"

/**
 * @file housing_demand_calculator.hpp
 * @brief Subsystem 155: Aggregates regional housing metrics into market pressure signals for AI actors.
 */

namespace Housing {

    /**
     * @struct RegionDemandStats
     * @brief Raw aggregated data pulled from the ECS by the Demographics system.
     */
    struct RegionDemandStats {
        uint32_t homeless_count{0};
        uint32_t overpop_count{0};
        uint32_t latent_demand{0}; // Housing-stressed citizens
        uint32_t vacant_units{0};  // Market Supply
        uint32_t total_units{1};   // Prevent div by 0
    };

    // --- Market Signal Events ---

    struct HousingDemandReportEvent {
        uint32_t region_id;
        float demand_pressure;
        float raw_demand;
    };

    struct DeveloperAIActivateEvent {
        uint32_t region_id;
        float demand_pressure;
    };

    struct EmergencyHousingTriggerEvent {
        uint32_t region_id;
        float demand_pressure;
    };

    /**
     * @class HousingDemandCalculator
     * @brief Computes weighted demand and fires economic triggers.
     */
    class HousingDemandCalculator {
    public:
        /**
         * @brief Calculates the weighted raw demand (the number of people actively seeking housing).
         */
        float calculate_raw_demand(const RegionDemandStats& stats) const {
            return static_cast<float>(stats.homeless_count) +
                  (static_cast<float>(stats.overpop_count) * 0.5f) +
                  (static_cast<float>(stats.latent_demand) * 0.3f);
        }

        /**
         * @brief Calculates the percentage of the regional housing market that is critically short.
         */
        float calculate_pressure(const RegionDemandStats& stats) const {
            float raw_demand = calculate_raw_demand(stats);
            float supply = static_cast<float>(stats.vacant_units);
            float max_units = static_cast<float>(std::max(1u, stats.total_units));

            return (raw_demand - supply) / max_units;
        }

        /**
         * @brief Processes a region's monthly stats and fires appropriate economic events.
         * @param publisher The Event Bus.
         * @param region_id The geographic zone being calculated.
         * @param stats The aggregated housing stats for this region.
         */
        void process_region(EventPublisher& publisher, uint32_t region_id, const RegionDemandStats& stats) {
            float raw = calculate_raw_demand(stats);
            float pressure = calculate_pressure(stats);

            // 1. Always publish the monthly baseline report for the UI and Analytics
            publisher.publish(HousingDemandReportEvent{region_id, pressure, raw});

            // 2. Evaluate triggers (Prioritize emergencies)
            if (pressure > 0.15f) {
                // > 15% shortage: Severe crisis.
                publisher.publish(EmergencyHousingTriggerEvent{region_id, pressure});
            }
            else if (pressure > 0.05f) {
                // > 5% shortage: Healthy market signal for private developers.
                publisher.publish(DeveloperAIActivateEvent{region_id, pressure});
            }
        }
    };
}