#pragma once

#include <cstdint>
#include <vector>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/terrain_type.hpp"
#include "../../core/04_Types/fixed_point.hpp"
#include "../05_Terrain/grid_data_store.hpp"
#include "seed_manager.hpp"

/**
 * @file resource_seeder.hpp
 * @brief Distributes economic resources across the world map using geologically-weighted probabilities.
 */

enum class ResourceType : uint8_t {
    None = 0,
    Timber,
    Coal,
    Iron,
    Oil,
    Uranium
};

namespace ResourceSeeder {

    /**
     * @brief Evaluates a terrain type and uses RNG to determine if a resource spawns.
     */
    constexpr ResourceType roll_for_resource(TerrainType terrain, Fixed32 roll) {
        switch (terrain) {
            case TerrainType::Mountain:
                if (roll < Fixed32(0.30f)) return ResourceType::Iron;
                if (roll < Fixed32(0.50f)) return ResourceType::Coal;
                if (roll < Fixed32(0.55f)) return ResourceType::Uranium;
                return ResourceType::None;

            case TerrainType::Hills:
                if (roll < Fixed32(0.20f)) return ResourceType::Coal;
                if (roll < Fixed32(0.35f)) return ResourceType::Iron;
                return ResourceType::None;

            case TerrainType::Desert:
                if (roll < Fixed32(0.15f)) return ResourceType::Oil;
                return ResourceType::None;

            case TerrainType::Forest:
                return ResourceType::Timber;

            case TerrainType::Ocean:
                if (roll < Fixed32(0.05f)) return ResourceType::Oil;
                return ResourceType::None;

            default:
                return ResourceType::None;
        }
    }

    /**
     * @brief Sweeps the entire world grid and populates it with harvestable resources.
     */
    inline void scatter_resources(GridDataStore& grid, SeedManager& seed_manager) {
        int32_t w = static_cast<int32_t>(grid.get_width());
        int32_t h = static_cast<int32_t>(grid.get_height());

        for (int32_t y = 0; y < h; ++y) {
            for (int32_t x = 0; x < w; ++x) {
                TileCoord current{x, y};
                GridCell& cell = grid.get_cell(current);

                // Strictly use Fixed32 to maintain cross-platform determinism
                Fixed32 roll = seed_manager.random_fixed(0.0f, 1.0f);

                ResourceType spawned_resource = roll_for_resource(cell.terrain, roll);

                // cell.resource_type = spawned_resource;
            }
        }
    }
}