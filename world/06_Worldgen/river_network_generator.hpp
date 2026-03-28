#pragma once

#include <vector>
#include <cstdint>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/fixed_point.hpp"
#include "../05_Terrain/elevation_layer.hpp"
#include "../05_Terrain/moisture_layer.hpp"
#include "../05_Terrain/water_body_layer.hpp"
#include "seed_manager.hpp"

/**
 * @file river_network_generator.hpp
 * @brief Orchestrates the procedural placement and routing of global river networks.
 */

namespace RiverNetworkGenerator {

    /**
     * @brief Generates a deterministic river network across the continent.
     * @param water The hydrological layer to be populated.
     * @param elevation Read-only topology map used to find high ground.
     * @param moisture Read-only moisture map used to find rainy/snowy regions.
     * @param seed_manager The world's master RNG for deterministic spring selection.
     * @param target_river_count The desired number of river source points.
     */
    inline void generate_networks(
        WaterBodyLayer& water,
        const ElevationLayer& elevation,
        const MoistureLayer& moisture,
        SeedManager& seed_manager,
        int32_t target_river_count)
    {
        int32_t w = static_cast<int32_t>(elevation.get_width());
        int32_t h = static_cast<int32_t>(elevation.get_height());

        std::vector<TileCoord> candidate_springs;

        // 1. Identify valid source points (Springs)
        // Rivers must start high in the mountains where there is sufficient moisture.
        Fixed32 min_elevation(0.6f);
        Fixed32 min_moisture(0.5f);

        for (int32_t y = 0; y < h; ++y) {
            for (int32_t x = 0; x < w; ++x) {
                TileCoord current{x, y};

                if (elevation.get_elevation(current) >= min_elevation &&
                    moisture.get_moisture(current) >= min_moisture) {
                    candidate_springs.push_back(current);
                }
            }
        }

        // If the map is a total desert or completely flat, no rivers form.
        if (candidate_springs.empty()) {
            return;
        }

        // 2. Spawn the rivers deterministically
        int32_t rivers_to_spawn = std::min(target_river_count, static_cast<int32_t>(candidate_springs.size()));

        for (int32_t i = 0; i < rivers_to_spawn; ++i) {
            // Pick a random candidate using our deterministic seed
            int32_t random_index = seed_manager.random_int(0, static_cast<int32_t>(candidate_springs.size()) - 1);
            TileCoord spring = candidate_springs[random_index];

            // Subsystem 55 takes over: trace steepest descent to the ocean
            // Starting width: 1.0. Growth per tile: 0.1 (simulating drainage accumulation).
            water.trace_river(spring, elevation, Fixed32(1.0f), Fixed32(0.1f));

            // Remove the chosen spring to prevent spawning two identical rivers on the same tile
            candidate_springs[random_index] = candidate_springs.back();
            candidate_springs.pop_back();
        }
    }
}