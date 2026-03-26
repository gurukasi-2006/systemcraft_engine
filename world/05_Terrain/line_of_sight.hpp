#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/fixed_point.hpp"
#include "../../core/04_Types/engine_assert.hpp"
#include "elevation_layer.hpp"

/**
 * @file line_of_sight.hpp
 * @brief High-performance raycasting utility for calculating visibility across topological terrain.
 */

namespace LineOfSight {

    /**
     * @brief Casts a 3D ray across the 2D grid to determine if terrain obstructs visibility.
     * @details Uses a modified Bresenham's Line Algorithm with Z-axis interpolation.
     * * @param start The origin coordinate (e.g., radar tower location).
     * @param end The target coordinate.
     * @param start_z The absolute elevation of the observer (terrain height + tower height).
     * @param end_z The absolute elevation of the target (terrain height + unit height).
     * @param elevation The topological map to check against.
     * @return true if the line of sight is clear, false if terrain blocks the ray.
     */
    constexpr bool check_visibility(
        TileCoord start,
        TileCoord end,
        Fixed32 start_z,
        Fixed32 end_z,
        const ElevationLayer& elevation)
    {
        // If start and end are the same tile, visibility is guaranteed
        if (start.x == end.x && start.y == end.y) {
            return true;
        }

        int32_t x0 = start.x;
        int32_t y0 = start.y;
        int32_t x1 = end.x;
        int32_t y1 = end.y;

        int32_t dx = std::abs(x1 - x0);
        int32_t dy = -std::abs(y1 - y0);
        int32_t sx = x0 < x1 ? 1 : -1;
        int32_t sy = y0 < y1 ? 1 : -1;
        int32_t err = dx + dy;

        // Calculate the maximum number of grid steps to interpolate the Z-axis accurately
        int32_t total_steps = std::max(std::abs(x1 - x0), std::abs(y1 - y0));
        Fixed32 current_z = start_z;
        Fixed32 z_step = (end_z - start_z) / Fixed32(static_cast<float>(total_steps));

        while (true) {
            TileCoord current{x0, y0};

            // Boundary safety check: if the ray leaves the map, it's a void.
            if (!elevation.is_valid(current)) {
                return false;
            }

            // --- THE OBSTRUCTION CHECK ---
            // If the terrain at this exact coordinate is taller than our ray's current altitude,
            // the signal/vision is physically blocked.
            if (elevation.get_elevation(current) > current_z) {
                return false;
            }

            // Have we reached the target?
            if (x0 == x1 && y0 == y1) {
                break;
            }

            // Step the ray algorithm forward
            int32_t e2 = 2 * err;
            if (e2 >= dy) {
                err += dy;
                x0 += sx;
            }
            if (e2 <= dx) {
                err += dx;
                y0 += sy;
            }

            // Interpolate height for the next tile evaluation
            current_z = current_z + z_step;
        }

        return true; // The ray successfully reached the target without hitting terrain
    }
}