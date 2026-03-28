#pragma once

#include <cstdint>
#include <vector>
#include <entt/entt.hpp>

#include "../../core/01_ECS_core/entity_manager.hpp"
#include "../../core/04_Types/tile_coord.hpp"
#include "starting_region_selector.hpp"
#include "seed_manager.hpp"

/**
 * @file population_seeder.hpp
 * @brief Bridges the world generator and the ECS by spawning the initial entities at highly-habitable coordinates.
 */

// --- Component Contracts ---
// Note: If you have a dedicated components header, move these there and #include it instead.
struct PositionComponent {
    TileCoord coord;
};

struct CitizenComponent {
    float hunger = 0.0f;
    float happiness = 100.0f;
    int32_t age = 18;
};

struct SettlementComponent {
    uint32_t population = 0;
    bool is_player_controlled = false;
};
// ---------------------------

namespace PopulationSeeder {

    /**
     * @brief Spawns settlements and citizens into the ECS at designated starting regions.
     * @param ecs Your custom Entity Manager for safe ID generation.
     * @param reg The master EnTT registry for component assignment.
     * @param regions The ranked list of viable starting locations.
     * @param citizens_per_settlement How many citizen entities to spawn per region.
     * @param rng The world's deterministic random number generator.
     */
    inline void seed_initial_populations(
        EntityManager& ecs,
        entt::registry& reg,
        const std::vector<StartingRegionSelector::CandidateRegion>& regions,
        int32_t citizens_per_settlement,
        SeedManager& rng)
    {
        for (size_t i = 0; i < regions.size(); ++i) {
            TileCoord spawn_loc = regions[i].coord;

            // 1. Spawn the Settlement Hub
            EntityID settlement_id = ecs.createEntity();
            entt::entity raw_settlement = static_cast<entt::entity>(settlement_id.raw_id);

            reg.emplace<PositionComponent>(raw_settlement, spawn_loc);

            // The first region in the ranked list is given to the Player. The rest belong to AI.
            bool is_player = (i == 0);
            reg.emplace<SettlementComponent>(raw_settlement, static_cast<uint32_t>(citizens_per_settlement), is_player);

            // 2. Spawn the Initial Citizens
            for (int32_t c = 0; c < citizens_per_settlement; ++c) {
                EntityID citizen_id = ecs.createEntity();
                entt::entity raw_citizen = static_cast<entt::entity>(citizen_id.raw_id);

                reg.emplace<PositionComponent>(raw_citizen, spawn_loc);

                // Add deterministic variety to starting citizens so they don't all die of old age on the same tick
                int32_t starting_age = rng.random_int(18, 35);
                reg.emplace<CitizenComponent>(raw_citizen, 0.0f, 100.0f, starting_age);
            }
        }
    }
}