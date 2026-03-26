#pragma once

#include "../../core/04_Types/terrain_type.hpp"
#include "../../core/04_Types/fixed_point.hpp"

/**
 * @file terrain_mapper.hpp
 * @brief Encapsulates the deterministic biome classification rules for Systemcraft.
 * @details Translates raw elevation and moisture values into discrete TerrainType enum values.
 */

/**
 * @namespace TerrainMapper
 * @brief Contains compile-time threshold evaluation for procedural terrain generation.
 */
namespace TerrainMapper {

    /**
     * @brief Determines the natural baseline terrain type for a given tile.
     * @details Uses a 2D matrix of elevation and moisture thresholds to evaluate the biome.
     * Evaluates strictly in fixed-point math to guarantee deterministic cross-platform map generation.
     * * @param elevation The topological height of the tile (-1.0 to 1.0).
     * @param moisture The water saturation level of the tile (0.0 to 1.0).
     * @return The classified TerrainType.
     */
    constexpr TerrainType determine_natural_terrain(Fixed32 elevation, Fixed32 moisture) {

        // Define elevation thresholds
        constexpr Fixed32 SEA_LEVEL(0.0f);
        constexpr Fixed32 COAST_LEVEL(0.1f);
        constexpr Fixed32 HILL_LEVEL(0.5f);
        constexpr Fixed32 MOUNTAIN_LEVEL(0.8f);

        // Define moisture thresholds
        constexpr Fixed32 ARID_LEVEL(0.2f);
        constexpr Fixed32 SEMI_ARID_LEVEL(0.4f);
        constexpr Fixed32 PLAINS_LEVEL(0.6f);
        constexpr Fixed32 GRASSLAND_LEVEL(0.8f);

        // 1. Evaluate Hydrological / Coastal Zones
        if (elevation <= SEA_LEVEL) {
            return TerrainType::Ocean;
        }
        if (elevation <= COAST_LEVEL) {
            return TerrainType::Coast;
        }

        // 2. Evaluate Elevated Zones
        if (elevation >= MOUNTAIN_LEVEL) {
            return TerrainType::Mountain;
        }
        if (elevation >= HILL_LEVEL) {
            return TerrainType::Hills;
        }

        // 3. Evaluate Lowland Biomes (between Coast and Hills)
        // Differentiated entirely by moisture availability
        if (moisture <= ARID_LEVEL) {
            return TerrainType::Desert;
        }
        if (moisture <= SEMI_ARID_LEVEL) {
            return TerrainType::SemiArid;
        }
        if (moisture <= PLAINS_LEVEL) {
            return TerrainType::Plains;
        }
        if (moisture <= GRASSLAND_LEVEL) {
            return TerrainType::Grassland;
        }

        // Default to Forest for high-moisture lowlands
        return TerrainType::Forest;
    }

}