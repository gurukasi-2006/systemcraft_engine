#pragma once

#include <vector>
#include <cstdint>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/fixed_point.hpp"
#include "../../core/04_Types/math_utils.hpp"
#include "../05_Terrain/elevation_layer.hpp"
#include "../05_Terrain/tile_neighbor_query.hpp"

/**
 * @file mountain_valley_former.hpp
 * @brief Geologically ages a procedural heightmap by amplifying peaks and eroding valleys.
 */

namespace MountainValleyFormer {

    /**
     * @brief Amplifies high-elevation terrain to create sharp, impassable peaks.
     * @param elevation The world's elevation layer (modified in place).
     * @param threshold The elevation point at which ridging begins (e.g., 0.6).
     * @param power The exponent multiplier for the amplification (e.g., 2.0).
     */
    inline void amplify_peaks(ElevationLayer& elevation, Fixed32 threshold, Fixed32 power) {
        int32_t w = static_cast<int32_t>(elevation.get_width());
        int32_t h = static_cast<int32_t>(elevation.get_height());

        for (int32_t y = 0; y < h; ++y) {
            for (int32_t x = 0; x < w; ++x) {
                TileCoord current{x, y};
                Fixed32 current_elev = elevation.get_elevation(current);

                if (current_elev > threshold) {
                    // Calculate how far above the threshold this tile is
                    Fixed32 excess = current_elev - threshold;

                    // Square the excess to pinch the peaks upward non-linearly
                    Fixed32 amplified = current_elev + (excess * excess * power);

                    // Safely clamp so we don't breach the atmosphere
                    elevation.set_elevation(current, MathUtils::clamp(amplified, Fixed32(-1.0f), Fixed32(1.0f)));
                }
            }
        }
    }

    /**
     * @brief Simulates basic hydraulic erosion by washing soil from steep inclines into local minima.
     * @param elevation The world's elevation layer (modified in place).
     * @param erosion_rate The fraction of height difference transferred per tick (e.g., 0.1).
     * @param iterations How many thousands of years of rainfall to simulate.
     */
    inline void erode_valleys(ElevationLayer& elevation, Fixed32 erosion_rate, int32_t iterations) {
        int32_t w = static_cast<int32_t>(elevation.get_width());
        int32_t h = static_cast<int32_t>(elevation.get_height());

        // Clamp the rate to prevent mathematical explosions (0.0 to 0.5 is safe)
        Fixed32 safe_rate = MathUtils::clamp(erosion_rate, Fixed32(0.0f), Fixed32(0.5f));

        for (int32_t i = 0; i < iterations; ++i) {
            // Delta buffer to store soil displacement without altering the current read-state
            std::vector<Fixed32> deltas(w * h, Fixed32(0));

            for (int32_t y = 0; y < h; ++y) {
                for (int32_t x = 0; x < w; ++x) {
                    TileCoord current{x, y};
                    Fixed32 current_elev = elevation.get_elevation(current);

                    // Find the steepest downward neighbor
                    auto neighbors = TileNeighborQuery::get_all(current, w, h);
                    TileCoord lowest_neighbor = current;
                    Fixed32 lowest_elev = current_elev;

                    for (int8_t n = 0; n < neighbors.count; ++n) {
                        Fixed32 n_elev = elevation.get_elevation(neighbors.coordinates[n]);
                        if (n_elev < lowest_elev) {
                            lowest_elev = n_elev;
                            lowest_neighbor = neighbors.coordinates[n];
                        }
                    }

                    // If a valid downhill slope exists, wash soil down it
                    if (lowest_elev < current_elev) {
                        Fixed32 diff = current_elev - lowest_elev;
                        Fixed32 transfer_amount = diff * safe_rate;

                        size_t current_idx = static_cast<size_t>(y) * w + static_cast<size_t>(x);
                        size_t neighbor_idx = static_cast<size_t>(lowest_neighbor.y) * w + static_cast<size_t>(lowest_neighbor.x);

                        // Subtract from the peak, add to the valley
                        deltas[current_idx] = deltas[current_idx] - transfer_amount;
                        deltas[neighbor_idx] = deltas[neighbor_idx] + transfer_amount;
                    }
                }
            }

            // Apply all deltas simultaneously (Double-buffer commit)
            for (int32_t y = 0; y < h; ++y) {
                for (int32_t x = 0; x < w; ++x) {
                    TileCoord current{x, y};
                    size_t idx = static_cast<size_t>(y) * w + static_cast<size_t>(x);

                    Fixed32 new_elev = elevation.get_elevation(current) + deltas[idx];
                    elevation.set_elevation(current, MathUtils::clamp(new_elev, Fixed32(-1.0f), Fixed32(1.0f)));
                }
            }
        }
    }
}