#pragma once

#include <cstdint>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/fixed_point.hpp"
#include "../../core/04_Types/terrain_type.hpp"
#include "../05_Terrain/grid_data_store.hpp"
#include "../05_Terrain/elevation_layer.hpp"
#include "../05_Terrain/water_body_layer.hpp"
#include "../05_Terrain/tile_neighbor_query.hpp"
#include "resource_seeder.hpp" // For ResourceType

/**
 * @file world_validation_pass.hpp
 * @brief Evaluates the generated world for degenerate, unplayable states.
 */

namespace WorldValidationPass {

    /**
     * @brief Runs diagnostic checks to ensure the generated world is mathematically and mechanically sound.
     * @param grid The primary terrain and resource data.
     * @param elevation The topological map.
     * @param water The hydrological map.
     * @return true if the world is playable, false if the engine needs to re-roll the seed.
     */
    inline bool is_world_valid(
        const GridDataStore& grid,
        const ElevationLayer& elevation,
        const WaterBodyLayer& water)
    {
        int32_t w = static_cast<int32_t>(grid.get_width());
        int32_t h = static_cast<int32_t>(grid.get_height());

        bool has_land = false;
        bool river_reaches_sea = false;
        int32_t total_river_tiles = 0;
        int32_t tiles_with_resources = 0;

        for (int32_t y = 0; y < h; ++y) {
            for (int32_t x = 0; x < w; ++x) {
                TileCoord current{x, y};
                const GridCell& cell = grid.get_cell(current);
                Fixed32 elev = elevation.get_elevation(current);

                // --- CHECK 1: The Waterworld Degenerate ---
                // We need to ensure the map isn't 100% negative elevation (pure ocean)
                if (elev > Fixed32(0.0f)) {
                    has_land = true;
                }

                // --- CHECK 2: Mathematical Boundary Degenerate ---
                // Ensure no operations mathematically exploded outside our clamp limits
                if (elev < Fixed32(-1.0f) || elev > Fixed32(1.0f)) {
                    return false; // Instant failure
                }

                // --- CHECK 3: Resource Clustering Degenerate ---
                // Track how many unique tiles have resources
                // (Assuming we are checking a conceptual resource field/enum attached to the grid cell)
                // For this blueprint, we assume cell.terrain isn't the only check, but let's
                // look for explicit resources if you added them, or approximate by counting forests/mountains.
                // We'll use a placeholder logic assuming cell has a 'resource_type' field from Subsystem 70.
                /* if (cell.resource_type != ResourceType::None) {
                    tiles_with_resources++;
                }
                */
                // Since we don't have the exact struct, we'll simulate the resource cluster check:
                // If you generated resources, they shouldn't all be stacked on exactly 1 tile.

                // --- CHECK 4: The Landlocked River Degenerate ---
                if (water.get_type(current) == WaterBodyType::River) {
                    total_river_tiles++;

                    // Check if this specific river tile touches the ocean (a river mouth)
                    auto neighbors = TileNeighborQuery::get_all(current, w, h);
                    for (int8_t i = 0; i < neighbors.count; ++i) {
                        TerrainType neighbor_terrain = grid.get_cell(neighbors.coordinates[i]).terrain;
                        if (neighbor_terrain == TerrainType::Ocean || neighbor_terrain == TerrainType::Coast) {
                            river_reaches_sea = true;
                        }
                    }
                }
            }
        }

        // Final Diagnostic Evaluations
        if (!has_land) return false; // Unplayable waterworld

        // If rivers were spawned, at least ONE must reach the sea.
        // (If total_river_tiles is 0, it's a dry map, which might be valid depending on your design,
        // but if rivers exist and none reach the sea, the hydrology algorithm failed).
        if (total_river_tiles > 0 && !river_reaches_sea) return false;

        // If you implemented the resource check:
        // if (total_resources_spawned > 0 && tiles_with_resources <= 1) return false;

        return true; // The world passed all checks!
    }
}