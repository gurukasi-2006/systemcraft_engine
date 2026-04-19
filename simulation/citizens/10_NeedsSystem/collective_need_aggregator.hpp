#pragma once

#include <entt/entt.hpp>
#include <unordered_map>
#include <array>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/time_constants.hpp"

#include "../10_NeedsSystem/need_definitions.hpp" // Subsystem 115
#include "../09_CitizenSpawner/citizen_spawner.hpp" // For PositionComponent

/**
 * @file collective_need_aggregator.hpp
 * @brief Subsystem 125: Monthly regional aggregation of individual need scores for government policy targeting.
 */

// We define RegionID here if it wasn't globally pulled in yet
namespace Population {
    using RegionID = uint32_t;
}

struct RegionNeedReportEvent {
    Population::RegionID region_id;
    std::array<float, static_cast<size_t>(Population::NeedType::COUNT)> average_needs;
    float vulnerable_pct; // Percentage of the regional population in a critical survival state (0.0 to 100.0)
};

class CollectiveNeedAggregator {
private:
    Population::RegionID get_region_for_tile(TileCoord coord) const {
        // Same spatial proxy used in Subsystem 112 (100x100 tile regions)
        return static_cast<Population::RegionID>(coord.x / 100);
    }

public:
    /**
     * @brief Aggregates demographic needs into a regional report once per month.
     * @param world The ECS master world.
     * @param publisher The event bus to broadcast the report to the Government.
     * @param current_tick The absolute simulation time.
     */
    void update(ECSWorld& world, EventPublisher& publisher, uint64_t current_tick) {
        constexpr uint64_t MONTH_TICKS = TimeConstants::TICKS_PER_YEAR / 12ULL;

        // Execute only exactly on the monthly rollover tick
        if (current_tick == 0 || current_tick % MONTH_TICKS != 0) {
            return;
        }

        // Internal struct to hold running sums before averaging
        struct RegionStats {
            uint32_t population{0};
            uint32_t vulnerable_count{0};
            std::array<float, static_cast<size_t>(Population::NeedType::COUNT)> need_sums{};
        };

        std::unordered_map<Population::RegionID, RegionStats> regional_data;

        auto view = world.registry.view<PositionComponent, Population::NeedsComponent>();

        // --- 1. Accumulate Data ---
        for (auto raw_id : view) {
            const auto& pos = view.get<PositionComponent>(raw_id);
            const auto& needs = view.get<Population::NeedsComponent>(raw_id);

            Population::RegionID region = get_region_for_tile(pos.coord);
            auto& stats = regional_data[region];

            stats.population++;

            bool is_vulnerable = false;

            for (size_t i = 0; i < static_cast<size_t>(Population::NeedType::COUNT); ++i) {
                float sat = needs.satisfaction_levels[i];
                stats.need_sums[i] += sat;

                // Mark citizen as vulnerable if their Food (index 0) drops below 40.0
                if (i == static_cast<size_t>(Population::NeedType::Food) && sat < 40.0f) {
                    is_vulnerable = true;
                }
            }

            if (is_vulnerable) {
                stats.vulnerable_count++;
            }
        }

        // --- 2. Calculate Averages and Publish Reports ---
        for (const auto& [region_id, stats] : regional_data) {
            if (stats.population == 0) continue;

            RegionNeedReportEvent report;
            report.region_id = region_id;

            // Calculate vulnerability percentage (0.0 to 100.0)
            report.vulnerable_pct = (static_cast<float>(stats.vulnerable_count) / static_cast<float>(stats.population)) * 100.0f;

            // Calculate means for all 13 needs
            for (size_t i = 0; i < static_cast<size_t>(Population::NeedType::COUNT); ++i) {
                report.average_needs[i] = stats.need_sums[i] / static_cast<float>(stats.population);
            }

            publisher.publish(report);
        }
    }
};