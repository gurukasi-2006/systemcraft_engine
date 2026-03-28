#pragma once

#include <cstdint>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/fixed_point.hpp"
#include "../../core/04_Types/terrain_type.hpp"
#include "../05_Terrain/elevation_layer.hpp"
#include "../05_Terrain/moisture_layer.hpp"
#include "../05_Terrain/grid_data_store.hpp"

/**
 * @file biome_assigner.hpp
 * @brief Maps raw elevation and moisture data into explicit geological biomes.
 * @details Utilizes a simplified Whittaker Biome model to guarantee logical ecosystem placement.
 */

namespace BiomeAssigner {

    /**
     * @brief Evaluates a single tile's physical properties to determine its biome.
     * @param elevation The height of the terrain (-1.0 to 1.0). Acts as a proxy for temperature.
     * @param moisture The water saturation level (0.0 to 1.0).
     * @return The specific TerrainType classification.
     */
    constexpr TerrainType determine_biome(Fixed32 elevation, Fixed32 moisture) {
        // 1. Absolute Thresholds (Overrides climate)
        if (elevation <= Fixed32(0.0f)) {
            return TerrainType::Ocean;
        }
        if (elevation <= Fixed32(0.05f)) {
            return TerrainType::Coast; // Sandy beaches right above sea level
        }

        // 2. High Altitude (Temperature drops drastically)
        if (elevation >= Fixed32(0.8f)) {
            return TerrainType::Mountain; // Too cold/steep for complex biomes
        }
        if (elevation >= Fixed32(0.5f)) {
            return TerrainType::Hills; // Rolling highlands
        }

        // 3. The Whittaker Matrix (Low/Mid elevation, categorized by moisture)
        if (moisture < Fixed32(0.3f)) {
            return TerrainType::Desert; // Hot and dry
        }
        else if (moisture < Fixed32(0.6f)) {
            return TerrainType::Plains; // Moderate temperature, moderate rain
        }
        else {
            return TerrainType::Forest; // Hot and wet (Jungle/Forest)
        }
    }

    /**
     * @brief Applies the biome classification across the entire procedural world.
     * @param grid The master data store where TerrainType is permanently recorded.
     * @param elevation Read-only topology data.
     * @param moisture Read-only hydrological data.
     */
    inline void apply_biomes(
        GridDataStore& grid,
        const ElevationLayer& elevation,
        const MoistureLayer& moisture)
    {
        int32_t w = static_cast<int32_t>(elevation.get_width());
        int32_t h = static_cast<int32_t>(elevation.get_height());

        for (int32_t y = 0; y < h; ++y) {
            for (int32_t x = 0; x < w; ++x) {
                TileCoord current{x, y};
                Fixed32 elev = elevation.get_elevation(current);
                Fixed32 moist = moisture.get_moisture(current); // Assuming get_moisture exists in MoistureLayer

                TerrainType biome = determine_biome(elev, moist);
                grid.get_cell(current).terrain = biome;
            }
        }
    }
}