#pragma once

#include <vector>
#include <cstdint>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/fixed_point.hpp"
#include "../../core/04_Types/engine_assert.hpp"
#include "elevation_layer.hpp"

/**
 * @file flood_layer.hpp
 * @brief Manages dynamic flooding events, including global sea-level rise and localized dam breaks.
 * @details Utilizes a high-speed uint8_t SoA to track flooded states without the overhead of std::vector<bool>.
 */

/**
 * @class FloodLayer
 * @brief A contiguous 1D array tracking the inundation state of the world grid.
 */
class FloodLayer {
private:
    uint32_t width_;
    uint32_t height_;

    /** * @brief High-performance boolean array (0 = dry, 1 = flooded).
     * @details We use uint8_t instead of bool because std::vector<bool> is a bit-packed proxy object
     * that severely degrades CPU cache performance and prevents compiler auto-vectorization.
     */
    std::vector<uint8_t> is_flooded_;

    /** @brief Tracks the current baseline global sea level. */
    Fixed32 global_sea_level_{0.0f};

    /**
     * @brief Converts a 2D coordinate into the 1D array index.
     */
    size_t get_index(TileCoord coord) const {
        return static_cast<size_t>(coord.y) * width_ + static_cast<size_t>(coord.x);
    }

public:
    /**
     * @brief Allocates the flood state memory upfront.
     * @param width The horizontal size of the map in tiles.
     * @param height The vertical size of the map in tiles.
     */
    FloodLayer(uint32_t width, uint32_t height)
        : width_(width), height_(height), is_flooded_(width * height, 0) {}

    /**
     * @brief Checks if a given coordinate is safely within the bounds of the map.
     */
    bool is_valid(TileCoord coord) const {
        return coord.x >= 0 && coord.x < static_cast<int32_t>(width_) &&
               coord.y >= 0 && coord.y < static_cast<int32_t>(height_);
    }

    /**
     * @brief Checks if a specific tile is currently underwater.
     */
    bool is_tile_flooded(TileCoord coord) const {
        ENGINE_ASSERT(is_valid(coord), "TileCoord out of bounds in FloodLayer.");
        return is_flooded_[get_index(coord)] == 1;
    }

    /**
     * @brief Simulates a global sea-level rise (e.g., climate change event).
     * @details Sweeps the entire map and floods any tile below the new virtual water level.
     * @param new_sea_level The new baseline elevation for the ocean.
     * @param elevation_layer Read-only reference to the world's topology.
     */
    void simulate_global_sea_level_rise(Fixed32 new_sea_level, const ElevationLayer& elevation_layer) {
        global_sea_level_ = new_sea_level;

        for (int32_t y = 0; y < static_cast<int32_t>(height_); ++y) {
            for (int32_t x = 0; x < static_cast<int32_t>(width_); ++x) {
                TileCoord current{x, y};
                if (elevation_layer.get_elevation(current) <= global_sea_level_) {
                    is_flooded_[get_index(current)] = 1;
                }
            }
        }
    }

    /**
     * @brief Simulates a localized flood event (e.g., dam break or flash flood).
     * @details Marks tiles within a specific radius as flooded if they fall below the surge elevation.
     * @param center The epicenter of the flood.
     * @param radius The maximum outward spread of the water in tiles.
     * @param surge_elevation The absolute height of the floodwater.
     * @param elevation_layer Read-only reference to the world's topology.
     */
    void simulate_localized_flood(TileCoord center, int32_t radius, Fixed32 surge_elevation, const ElevationLayer& elevation_layer) {
        int32_t start_x = std::max(0, center.x - radius);
        int32_t end_x = std::min(static_cast<int32_t>(width_) - 1, center.x + radius);
        int32_t start_y = std::max(0, center.y - radius);
        int32_t end_y = std::min(static_cast<int32_t>(height_) - 1, center.y + radius);

        for (int32_t y = start_y; y <= end_y; ++y) {
            for (int32_t x = start_x; x <= end_x; ++x) {
                TileCoord current{x, y};
                // Only flood if the terrain is lower than the surge water level
                if (elevation_layer.get_elevation(current) <= surge_elevation) {
                    is_flooded_[get_index(current)] = 1;
                }
            }
        }
    }

    Fixed32 get_global_sea_level() const { return global_sea_level_; }
};