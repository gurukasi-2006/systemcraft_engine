#pragma once

#include <array>
#include <unordered_map>
#include <entt/entt.hpp>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/04_Types/time_constants.hpp"
#include "../../../core/04_Types/tile_coord.hpp"

#include "../08_Citizencomponent/age_lifecycle_component.hpp"
#include "../08_Citizencomponent/employment_component.hpp"
#include "../09_CitizenSpawner/citizen_spawner.hpp" // For PositionComponent

/**
 * @file population_snapshot.hpp
 * @brief Subsystem 114: Aggregates demographic data into a fixed-size ring buffer for UI and analytics.
 */

namespace Population {

    /**
     * @struct SnapshotData
     * @brief A single monthly record of the nation's demographics.
     */
    struct SnapshotData {
        uint64_t tick_recorded{0};

        uint32_t total_population{0};

        // Age Brackets
        uint32_t children{0};
        uint32_t adults{0};
        uint32_t elders{0};

        // Employment Status
        uint32_t employed{0};
        uint32_t unemployed{0};
        uint32_t students{0};
        uint32_t retired{0};

        // Geographic Distribution
        std::unordered_map<RegionID, uint32_t> regional_population;
    };

    class PopulationSnapshotSystem {
    public:
        // 24 entries = exactly 2 game-years of monthly history
        static constexpr size_t RING_SIZE = 24;

        std::array<SnapshotData, RING_SIZE> ring_buffer;
        size_t head{0};

        /**
         * @brief Evaluates the current tick and dumps an aggregate if a month has passed.
         */
        void update(ECSWorld& world, uint64_t current_tick) {
            // Only execute exactly on the monthly rollover tick
            // (Assuming TICKS_PER_MONTH is defined. If not, fallback to TICKS_PER_YEAR / 12)
            constexpr uint64_t MONTH_TICKS = TimeConstants::TICKS_PER_YEAR / 12ULL;

            if (current_tick > 0 && current_tick % MONTH_TICKS == 0) {
                SnapshotData snapshot = aggregate_by_age_and_status(world, current_tick);

                // Write to the ring buffer, overwriting the oldest entry if looping
                ring_buffer[head % RING_SIZE] = snapshot;
                head++;
            }
        }

        /**
         * @brief Fetches the most recent demographic snapshot. O(1) lookup.
         */
        const SnapshotData& get_latest_snapshot() const {
            if (head == 0) return ring_buffer[0]; // Empty/Zeroed state
            return ring_buffer[(head - 1) % RING_SIZE];
        }

    private:
        SnapshotData aggregate_by_age_and_status(ECSWorld& world, uint64_t current_tick) {
            SnapshotData snap;
            snap.tick_recorded = current_tick;

            auto view = world.registry.view<
                Population::AgeLifecycleComponent,
                Population::EmploymentComponent,
                PositionComponent
            >();

            for (auto raw_id : view) {
                snap.total_population++;

                auto& age = view.get<Population::AgeLifecycleComponent>(raw_id);
                auto& emp = view.get<Population::EmploymentComponent>(raw_id);
                auto& pos = view.get<PositionComponent>(raw_id);

                // --- 1. Age Aggregation ---
                if (age.stage == Population::LifeStage::Child) snap.children++;
                else if (age.stage == Population::LifeStage::Adult) snap.adults++;
                else snap.elders++;

                // --- 2. Employment Aggregation ---
                if (emp.status == Population::EmploymentStatus::Employed) snap.employed++;
                else if (emp.status == Population::EmploymentStatus::Unemployed) snap.unemployed++;
                else if (emp.status == Population::EmploymentStatus::Student) snap.students++;
                else if (emp.status == Population::EmploymentStatus::Retired) snap.retired++;

                // --- 3. Regional Aggregation ---
                RegionID region = static_cast<RegionID>(pos.coord.x / 100);
                snap.regional_population[region]++;
            }

            return snap;
        }
    };
}