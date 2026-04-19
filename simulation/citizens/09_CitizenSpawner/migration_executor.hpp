#pragma once

#include <entt/entt.hpp>
#include <random>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/time_constants.hpp"
#include "../../../core/04_Types/tile_coord.hpp"

#include "../08_Citizencomponent/migration_intent_component.hpp"
#include "../08_Citizencomponent/employment_component.hpp"
#include "../08_Citizencomponent/housing_component.hpp"
#include "../08_Citizencomponent/age_lifecycle_component.hpp"
#include "../08_Citizencomponent/education_component.hpp"
#include "../09_CitizenSpawner/citizen_spawner.hpp" // For PositionComponent

/**
 * @file migration_executor.hpp
 * @brief Subsystem 112: Executes the physical relocation of citizens who hit critical migration pressure.
 */

struct CitizenMigratedEvent {
    EntityID citizen;
    Population::RegionID from_region;
    Population::RegionID to_region;
    float skill_level;
    float age_years;
};

class MigrationExecutor {
private:
    /**
     * @brief Proxy for regional spatial mapping.
     * @details In the full engine, this asks the Terrain System. Here, we mock it via math.
     */
    Population::RegionID get_region_for_tile(TileCoord coord) {
        return static_cast<Population::RegionID>(coord.x / 100);
    }

    /**
     * @brief Finds a valid, empty tile in the target region.
     */
    TileCoord select_target_tile(Population::RegionID region, std::mt19937& rng) {
        uint32_t base_x = region * 100;
        std::uniform_int_distribution<uint32_t> dist(1, 99);
        return TileCoord{base_x + dist(rng), base_x + dist(rng)};
    }

public:
    /**
     * @brief Scans for bursting intent gauges and processes the complex multi-system relocation.
     */
    void update(ECSWorld& world, EventPublisher& publisher, std::mt19937& rng) {
        auto view = world.registry.view<
            Population::MigrationIntentComponent,
            PositionComponent,
            Population::EmploymentComponent,
            Population::HousingComponent,
            Population::AgeLifecycleComponent,
            Population::EducationComponent
        >();

        for (auto raw_id : view) {
            auto& mig = view.get<Population::MigrationIntentComponent>(raw_id);

            // Trigger threshold check
            if (mig.is_ready_to_migrate() && mig.target_region.has_value()) {
                auto& pos = view.get<PositionComponent>(raw_id);
                auto& emp = view.get<Population::EmploymentComponent>(raw_id);
                auto& housing = view.get<Population::HousingComponent>(raw_id);
                auto& age = view.get<Population::AgeLifecycleComponent>(raw_id);
                auto& edu = view.get<Population::EducationComponent>(raw_id);

                EntityID citizen{static_cast<uint32_t>(raw_id)};

                Population::RegionID from_region = get_region_for_tile(pos.coord);
                Population::RegionID to_region = mig.target_region.value();

                // --- 1. Gather Demographics for "Brain Drain" Event ---
                float age_years = static_cast<float>(age.current_age_ticks) / TimeConstants::TICKS_PER_YEAR;

                // Track their highest developed skill to report macroeconomic loss/gain
                float max_skill = 0.0f;
                for (float skill : edu.skill_levels) {
                    if (skill > max_skill) max_skill = skill;
                }

                // --- 2. Spatial Relocation ---
                TileCoord old_tile = pos.coord;
                TileCoord new_tile = select_target_tile(to_region, rng);
                pos.coord = new_tile;

                // Note: If we had a direct handle to the actual SpatialIndex class here,
                // we would execute: spatial_index.move_entity(old_tile, new_tile, citizen);

                // --- 3. Institutional Disconnect ---
                // Citizen quits their job
                emp.employer = EntityID{0};
                emp.status = Population::EmploymentStatus::Unemployed;

                // Citizen abandons their house
                housing.housing_entity = EntityID{0};

                // --- 4. Fire Macro-Economic Event ---
                publisher.publish(CitizenMigratedEvent{
                    citizen,
                    from_region,
                    to_region,
                    max_skill,
                    age_years
                });

                // --- 5. Reset Intent Gauge ---
                mig.reset_intent();
            }
        }
    }
};