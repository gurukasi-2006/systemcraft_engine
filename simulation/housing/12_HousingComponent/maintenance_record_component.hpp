#pragma once

#include <cstdint>
#include <array>

#include "../../../core/04_Types/entity_id.hpp"
#include "../../../core/04_Types/time_constants.hpp"

/**
 * @file maintenance_record_component.hpp
 * @brief Subsystem 148: Logs contractor events and calculates escalating maintenance frequencies for aging infrastructure.
 */

namespace Housing {

    /**
     * @struct MaintenanceEvent
     * @brief A single ledger entry for a repair job.
     */
    struct MaintenanceEvent {
        uint64_t tick{0};
        EntityID contractor{0};
    };

    /**
     * @struct MaintenanceRecordComponent
     * @brief Tracks the repair history of a building and schedules its next maintenance window.
     */
    struct MaintenanceRecordComponent {
        uint64_t last_maintenance_tick{0};
        EntityID last_contractor{0};
        uint64_t next_scheduled_tick{0};

        // Circular buffer for the last 6 maintenance events
        std::array<MaintenanceEvent, 6> history{};
        size_t history_head{0};

        /**
         * @brief Logs a new maintenance event and dynamically schedules the next one.
         * @param current_tick The absolute simulation time right now.
         * @param contractor The ID of the crew that performed the repairs.
         * @param construction_tick The tick this building was originally finished.
         */
        void record_maintenance(uint64_t current_tick, EntityID contractor, uint64_t construction_tick) {
            // 1. Update active trackers
            last_maintenance_tick = current_tick;
            last_contractor = contractor;

            // 2. Append to circular history log
            history[history_head] = MaintenanceEvent{current_tick, contractor};
            history_head = (history_head + 1) % 6;

            // 3. Calculate accelerating infrastructure debt
            schedule_next(current_tick, construction_tick);
        }

        /**
         * @brief Calculates the dynamic maintenance interval based on building age.
         */
        void schedule_next(uint64_t current_tick, uint64_t construction_tick) {
            float age_years = 0.0f;
            if (current_tick > construction_tick) {
                age_years = static_cast<float>(current_tick - construction_tick) / static_cast<float>(TimeConstants::TICKS_PER_YEAR);
            }

            // Formula: maintenance_interval = TICKS_PER_YEAR / (0.5 + building_age_years * 0.1)
            float interval_denominator = 0.5f + (age_years * 0.1f);
            float interval_ticks = static_cast<float>(TimeConstants::TICKS_PER_YEAR) / interval_denominator;

            next_scheduled_tick = current_tick + static_cast<uint64_t>(interval_ticks);
        }
    };
}