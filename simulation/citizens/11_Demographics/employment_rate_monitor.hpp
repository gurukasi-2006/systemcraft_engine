#pragma once

#include <entt/entt.hpp>
#include <unordered_map>
#include <vector>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/tile_coord.hpp"
#include "../../../core/04_Types/time_constants.hpp"
#include "../../../core/04_Types/economic_constants.hpp"
#include "../08_Citizencomponent/employment_component.hpp"
#include "../09_CitizenSpawner/citizen_spawner.hpp" // For PositionComponent

/**
 * @file employment_rate_monitor.hpp
 * @brief Subsystem 133: Calculates ILO-standard unemployment rates and triggers NAIRU recession warnings.
 */

namespace Labour {
    //constexpr float NATURAL_UNEMPLOYMENT_RATE = 0.05f; // 5% base structural unemployment
}

namespace Demographics {

    /**
     * @struct RegionalEmploymentData
     * @brief Holds the employment demographic breakdown for a specific region.
     */
    struct RegionalEmploymentData {
        uint32_t employed_count{0};
        uint32_t unemployed_count{0};
        uint32_t student_count{0};
        uint32_t retired_count{0};

        uint32_t labor_force{0};
        float official_unemployment_rate{0.0f};
        float nairu_gap{0.0f};
    };

    /**
     * @struct EmploymentDemographicsEvent
     * @brief The monthly statistical packet sent to the Economy and UI systems.
     */
    struct EmploymentDemographicsEvent {
        std::unordered_map<uint32_t, RegionalEmploymentData> regional_data;
    };

    /**
     * @struct RecessionWarningEvent
     * @brief Fired when unemployment severely breaches the NAIRU threshold.
     */
    struct RecessionWarningEvent {
        uint32_t region_id;
        float severity; // The extent to which unemployment exceeds the NAIRU gap tolerance
    };

    class EmploymentRateMonitor {
    private:
        uint32_t get_region_for_tile(TileCoord coord) const {
            return static_cast<uint32_t>(coord.x / 100);
        }

    public:
        /**
         * @brief Scans the population monthly to calculate labor force metrics and detect recessions.
         */
        void update(ECSWorld& world, EventPublisher& publisher, uint64_t current_tick) {
            constexpr uint64_t MONTH_TICKS = TimeConstants::TICKS_PER_YEAR / 12ULL;

            // Execute only on the monthly rollover tick
            if (current_tick == 0 || current_tick % MONTH_TICKS != 0) {
                return;
            }

            auto view = world.registry.view<PositionComponent, Population::EmploymentComponent>();

            EmploymentDemographicsEvent report;

            // --- 1. Count Statuses per Region ---
            for (auto raw_id : view) {
                const auto& pos = view.get<PositionComponent>(raw_id);
                const auto& emp = view.get<Population::EmploymentComponent>(raw_id);

                uint32_t region = get_region_for_tile(pos.coord);
                auto& data = report.regional_data[region];

                switch (emp.status) {
                    case Population::EmploymentStatus::Employed:   data.employed_count++; break;
                    case Population::EmploymentStatus::Unemployed: data.unemployed_count++; break;
                    case Population::EmploymentStatus::Student:    data.student_count++; break;
                    case Population::EmploymentStatus::Retired:    data.retired_count++; break;
                }
            }

            // --- 2. Calculate ILO Metrics and NAIRU Gap ---
            for (auto& [region, data] : report.regional_data) {
                // ILO Methodology: Labor force explicitly excludes Students and Retirees
                data.labor_force = data.employed_count + data.unemployed_count;

                if (data.labor_force > 0) {
                    data.official_unemployment_rate = static_cast<float>(data.unemployed_count) / static_cast<float>(data.labor_force);
                    data.nairu_gap = data.official_unemployment_rate - Labour::NATURAL_UNEMPLOYMENT_RATE;

                    // --- 3. Trigger Policy Events ---
                    // If unemployment is 3% above the natural rate (e.g., UR > 8% when NAIRU is 5%)
                    if (data.nairu_gap > 0.03f) {
                        publisher.publish(RecessionWarningEvent{
                            region,
                            data.nairu_gap
                        });
                    }
                }
            }

            // --- 4. Broadcast Monthly Stats to UI ---
            publisher.publish(report);
        }
    };
}