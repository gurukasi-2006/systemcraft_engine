#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/fixed_point.hpp"
#include "../../core/04_Types/math_utils.hpp"

/**
 * @file continent_sculptor.hpp
 * @brief Applies macro-geographical masks to raw noise to shape oceans and landmasses.
 */

/**
 * @enum ContinentShape
 * @brief The macro-layout of the generated world.
 */
enum class ContinentShape : uint8_t {
    Pangea,         ///< One massive central supercontinent surrounded by ocean.
    TwoContinents,  ///< Two large landmasses split by a central ocean.
    Archipelago,    ///< Scattered islands; overall sea level is artificially raised.
    Infinite        ///< No mask applied; raw noise extends to the borders.
};

namespace ContinentSculptor {

    /**
     * @brief Modifies a raw noise elevation to conform to a specific continental shape.
     * @param coord The target grid location.
     * @param map_width Total map width in tiles.
     * @param map_height Total map height in tiles.
     * @param raw_elevation The unmasked fBm noise elevation (-1.0 to 1.0).
     * @param shape The desired macro-geography.
     * @return The sculpted elevation, clamped safely between -1.0 and 1.0.
     */
    inline Fixed32 apply_mask(
        TileCoord coord,
        int32_t map_width,
        int32_t map_height,
        Fixed32 raw_elevation,
        ContinentShape shape)
    {
        if (shape == ContinentShape::Infinite) {
            return raw_elevation; // Bypass masking completely
        }

        // Fast float conversions for distance math
        float x = static_cast<float>(coord.x);
        float y = static_cast<float>(coord.y);
        float w = static_cast<float>(map_width);
        float h = static_cast<float>(map_height);

        float mask_modifier = 0.0f;

        if (shape == ContinentShape::Pangea) {
            // 1. Calculate distance from the exact center of the map
            float cx = w * 0.5f;
            float cy = h * 0.5f;

            // 2. Normalize coordinates from -1.0 to 1.0 relative to the center
            float nx = (x - cx) / cx;
            float ny = (y - cy) / cy;

            // 3. Calculate distance squared (saves a slow std::sqrt call)
            // Center is 0.0, corners are 2.0
            float distance_sq = (nx * nx) + (ny * ny);

            // 4. Create a drop-off curve.
            // By multiplying distance_sq, we aggressively push the edges down while leaving the center mostly untouched.
            mask_modifier = distance_sq * 1.5f;
        }
        else if (shape == ContinentShape::TwoContinents) {
            // Create two focal points (West and East)
            float cx1 = w * 0.25f;
            float cx2 = w * 0.75f;
            float cy = h * 0.5f;

            // Find distance to the closest focal point
            float nx1 = (x - cx1) / (w * 0.5f);
            float nx2 = (x - cx2) / (w * 0.5f);
            float ny = (y - cy) / cy;

            float dist_sq_1 = (nx1 * nx1) + (ny * ny);
            float dist_sq_2 = (nx2 * nx2) + (ny * ny);

            float min_dist_sq = std::min(dist_sq_1, dist_sq_2);
            mask_modifier = min_dist_sq * 1.8f;
        }
        else if (shape == ContinentShape::Archipelago) {
            // For islands, we don't need a radial mask, we just sink the entire world
            // uniformly so only the tallest peaks of the noise break the surface.
            mask_modifier = 0.6f;
        }

        // Subtract the mask modifier from the raw elevation
        Fixed32 sculpted_elevation = raw_elevation - Fixed32(mask_modifier);

        // Clamp securely to engine limits
        return MathUtils::clamp(sculpted_elevation, Fixed32(-1.0f), Fixed32(1.0f));
    }
}