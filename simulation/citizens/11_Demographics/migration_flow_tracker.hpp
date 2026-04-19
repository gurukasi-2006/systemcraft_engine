#pragma once

#include <entt/entt.hpp>
#include <unordered_map>
#include <vector>
#include <cmath>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/03_Event_Bus/subscriber_registry.hpp"
#include "../../../core/04_Types/time_constants.hpp"

#include "../08_Citizencomponent/education_component.hpp"
#include "../09_CitizenSpawner/migration_executor.hpp" // Subsystem 108: CitizenMigratedEvent

/**
 * @file migration_flow_tracker.hpp
 * @brief Subsystem 135: Records skill-weighted migration to identify chronic "hollow migration" and Brain Drain.
 */

namespace Demographics {

    /**
     * @struct BrainDrainAlertEvent
     * @brief Fired when a region suffers a catastrophic net loss of skilled labor.
     */
    struct BrainDrainAlertEvent {
        uint32_t region_id;
        float severity; // Absolute value of the negative skill balance
    };

    /**
     * @struct RegionalMigrationStats
     * @brief Internal accumulator for annual flow tracking.
     */
    struct RegionalMigrationStats {
        uint32_t inflow_count{0};
        float inflow_skill_sum{0.0f};

        uint32_t outflow_count{0};
        float outflow_skill_sum{0.0f};
    };

    class MigrationFlowTracker {
    private:
        std::vector<CitizenMigratedEvent> event_queue_;
        std::unordered_map<uint32_t, RegionalMigrationStats> annual_stats_;

        /**
         * @brief Safely extracts the maximum skill level of a citizen.
         */
        float get_citizen_skill(ECSWorld& world, EntityID citizen) {
            entt::entity raw = static_cast<entt::entity>(citizen.raw_id);
            if (world.registry.valid(raw) && world.registry.all_of<Population::EducationComponent>(raw)) {
                const auto& edu = world.registry.get<Population::EducationComponent>(raw);
                float max_skill = 0.0f;
                for (float s : edu.skill_levels) {
                    if (s > max_skill) max_skill = s;
                }
                return max_skill;
            }
            return 0.0f;
        }

    public:
        /**
         * @brief Binds to the Event Bus to track migrations as they happen.
         */
        MigrationFlowTracker(SubscriberRegistry& registry) {
            registry.subscribe<CitizenMigratedEvent>([this](const CitizenMigratedEvent& ev) {
                event_queue_.push_back(ev);
            });
        }

        /**
         * @brief Processes queued events and evaluates annual Brain Drain.
         */
        void update(ECSWorld& world, EventPublisher& publisher, uint64_t current_tick) {
            // --- 1. Process Event Queue ---
            // We do this every tick to prevent the queue from growing too large over a year,
            // and to ensure we read the citizen's skill BEFORE they potentially die.
            for (const auto& ev : event_queue_) {
                float skill = get_citizen_skill(world, ev.citizen);

                annual_stats_[ev.from_region].outflow_count++;
                annual_stats_[ev.from_region].outflow_skill_sum += skill;

                annual_stats_[ev.to_region].inflow_count++;
                annual_stats_[ev.to_region].inflow_skill_sum += skill;
            }
            event_queue_.clear();

            // --- 2. Annual Brain Drain Evaluation ---
            constexpr uint64_t YEAR_TICKS = TimeConstants::TICKS_PER_YEAR;
            if (current_tick == 0 || current_tick % YEAR_TICKS != 0) {
                return;
            }

            for (auto& [region, stats] : annual_stats_) {
                float avg_inflow = stats.inflow_count > 0 ? (stats.inflow_skill_sum / static_cast<float>(stats.inflow_count)) : 0.0f;
                float avg_outflow = stats.outflow_count > 0 ? (stats.outflow_skill_sum / static_cast<float>(stats.outflow_count)) : 0.0f;

                // Only evaluate regions that are actually losing citizens
                if (stats.outflow_count > 0) {
                    float skill_balance = avg_inflow - avg_outflow;

                    // Trigger the policy crisis if balance drops below -1.5
                    if (skill_balance < -1.5f) {
                        publisher.publish(BrainDrainAlertEvent{
                            region,
                            std::abs(skill_balance)
                        });
                    }
                }
            }

            // --- 3. Reset Annual Accumulators ---
            annual_stats_.clear();
        }
    };
}