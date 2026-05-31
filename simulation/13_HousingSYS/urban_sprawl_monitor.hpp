#pragma once

#include <cstdint>
#include <vector>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"

// Housing Components
#include "../12_HousingComponent/floor_area_density_component.hpp"
#include "../12_HousingComponent/occupancy_component.hpp"

/**
 * @file urban_sprawl_monitor.hpp
 * @brief Subsystem 167: Tracks housing density ratios and penalizes sprawling infrastructure costs.
 */

namespace Housing {

    // --- Events ---

    struct SprawlWarningEvent {
        uint32_t region_id;
        float sprawl_ratio;
        float road_cost_multiplier;
        float utility_cost_multiplier;
    };

    /**
     * @struct SprawlMetrics
     * @brief The aggregated output of a regional sprawl survey.
     */
    struct SprawlMetrics {
        uint32_t low_density_units{0};
        uint32_t high_density_units{0};
        uint32_t total_units{0};
        float sprawl_ratio{0.0f};

        float road_cost_multiplier{1.0f};
        float utility_cost_multiplier{1.0f};
    };

    /**
     * @class UrbanSprawlMonitor
     * @brief Evaluates regional housing to calculate fiscal multipliers based on density.
     */
    class UrbanSprawlMonitor {
    public:
        /**
         * @brief Scans the city and calculates the fiscal penalties of sprawl.
         * @param region_id The geographic zone being surveyed.
         */
        SprawlMetrics evaluate_region(ECSWorld& world, EventPublisher& publisher, uint32_t region_id) {
            SprawlMetrics metrics;

            auto view = world.registry.view<FloorAreaDensityComponent, OccupancyComponent>();

            for (auto raw : view) {
                const auto& density = view.get<FloorAreaDensityComponent>(raw);
                const auto& occ = view.get<OccupancyComponent>(raw);

                // Use max_capacity as the metric for "units" (households) in the building
                uint32_t units = occ.max_capacity;
                metrics.total_units += units;

                // Thresholds from design doc: Low Density is FAR < 0.8
                if (density.floor_area_ratio <= 0.8f) {
                    metrics.low_density_units += units;
                } else if (density.floor_area_ratio >= 2.0f) {
                    metrics.high_density_units += units;
                }
            }

            if (metrics.total_units > 0) {
                metrics.sprawl_ratio = static_cast<float>(metrics.low_density_units) / static_cast<float>(metrics.total_units);
            }

            // Infrastructure cost penalties from sprawl
            // road_cost = base * (1 + sprawl_ratio * 1.5)
            // utility_cost = base * (1 + sprawl_ratio * 0.8)
            metrics.road_cost_multiplier = 1.0f + (metrics.sprawl_ratio * 1.5f);
            metrics.utility_cost_multiplier = 1.0f + (metrics.sprawl_ratio * 0.8f);

            // Trigger warnings for > 70% Sprawl
            if (metrics.sprawl_ratio > 0.70f) {
                publisher.publish(SprawlWarningEvent{
                    region_id,
                    metrics.sprawl_ratio,
                    metrics.road_cost_multiplier,
                    metrics.utility_cost_multiplier
                });
            }

            return metrics;
        }
    };
}