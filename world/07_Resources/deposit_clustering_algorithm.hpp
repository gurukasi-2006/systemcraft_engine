#pragma once

#include <vector>
#include <queue>
#include <cstdint>
#include <entt/entt.hpp>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/fixed_point.hpp"
#include "../../core/01_ECS_core/entity_manager.hpp"
#include "../05_Terrain/grid_data_store.hpp"
#include "../05_Terrain/tile_neighbor_query.hpp"
#include "../06_Worldgen/seed_manager.hpp"
#include "../06_Worldgen/population_seeder.hpp" // For PositionComponent
#include "resource_deposit_component.hpp"
#include "deposit_registry.hpp"

/**
 * @file deposit_clustering_algorithm.hpp
 * @brief Expands isolated resource seeds into contiguous, geologically logical clusters.
 */

namespace Resources {

    class DepositClusteringAlgorithm {
    public:
        /**
         * @brief Scans the ECS for initial resource seeds and flood-fills them outward.
         * @param ecs The master Entity Manager.
         * @param reg The EnTT registry containing the seeds.
         * @param deposit_index The spatial lookup index (updated as new deposits spawn).
         * @param grid The world map (used to enforce biome boundaries).
         * @param rng The deterministic random number generator.
         * @param max_cluster_size The maximum number of tiles a single vein can occupy.
         */
        static void expand_clusters(
            EntityManager& ecs,
            entt::registry& reg,
            DepositRegistry& deposit_index,
            const GridDataStore& grid,
            SeedManager& rng,
            int32_t max_cluster_size = 5)
        {
            uint32_t w = grid.get_width();
            uint32_t h = grid.get_height();

            // 1. Identify all initial seeds
            // We copy the view to a vector because we will be creating new entities during the loop
            std::vector<entt::entity> seeds;
            for (auto entity : reg.view<ResourceDepositComponent, PositionComponent>()) {
                seeds.push_back(entity);
            }

            // Global visited array to prevent clusters from bleeding into each other
            std::vector<bool> global_visited(static_cast<size_t>(w) * h, false);
            auto get_idx = [&](TileCoord c) {
                return static_cast<size_t>(c.y) * w + static_cast<size_t>(c.x);
            };

            // Mark all initial seeds as visited
            for (auto entity : seeds) {
                global_visited[get_idx(reg.get<PositionComponent>(entity).coord)] = true;
            }

            // 2. Expand each seed using a Breadth-First Search (Flood Fill)
            for (auto seed_entity : seeds) {
                auto& seed_deposit = reg.get<ResourceDepositComponent>(seed_entity);
                auto& seed_pos = reg.get<PositionComponent>(seed_entity);
                TerrainType seed_terrain = grid.get_cell(seed_pos.coord).terrain;

                std::queue<TileCoord> q;
                q.push(seed_pos.coord);

                int32_t current_cluster_size = 1;

                while (!q.empty() && current_cluster_size < max_cluster_size) {
                    TileCoord current = q.front();
                    q.pop();

                    auto neighbors = TileNeighborQuery::get_all(current, static_cast<int32_t>(w), static_cast<int32_t>(h));

                    for (int8_t i = 0; i < neighbors.count; ++i) {
                        TileCoord n = neighbors.coordinates[i];
                        size_t n_idx = get_idx(n);

                        if (global_visited[n_idx]) continue;
                        global_visited[n_idx] = true;

                        // Geological coherence constraint: Veins cannot cross biome borders
                        if (grid.get_cell(n).terrain != seed_terrain) continue;

                        // Seeded flood fill: Roll RNG to determine if the vein spreads here
                        // E.g., 60% chance to spread to a valid neighbor
                        if (rng.random_fixed(0.0f, 1.0f) < Fixed32(0.60f)) {

                            // Spawn the new deposit chunk
                            EntityID new_id = ecs.createEntity();
                            entt::entity raw_id = static_cast<entt::entity>(new_id.raw_id);

                            reg.emplace<PositionComponent>(raw_id, PositionComponent{n});

                            // Inherit the properties of the parent seed
                            reg.emplace<ResourceDepositComponent>(
                                raw_id,
                                seed_deposit.type,
                                seed_deposit.total_reserve_quantity,
                                seed_deposit.extraction_difficulty
                            );

                            deposit_index.register_deposit(n, new_id);
                            q.push(n);

                            current_cluster_size++;
                            if (current_cluster_size >= max_cluster_size) break;
                        }
                    }
                }
            }
        }
    };
}