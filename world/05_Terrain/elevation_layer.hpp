#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/fixed_point.hpp"
#include "../../core/04_Types/engine_assert.hpp"
#include "../../core/04_Types/math_utils.hpp"

/**
 * @file elevation_layer.hpp
 * @brief Manages the topological height map of the simulation.
 * @details Implemented as a Structure of Arrays (SoA) parallel layer to maximize CPU cache utilization during line-of-sight, water flow, and gradient queries.
 */

/**
 * @class ElevationLayer
 * @brief A strictly contiguous 1D array of fixed-point elevation values.
 */
class ElevationLayer {
private:
    uint32_t width_;
    uint32_t height_;

    /** @brief The isolated array of Fixed32 elevation values. */
    std::vector<Fixed32> elevation_data_;

    /**
     * @brief Converts a 2D coordinate into the 1D array index.
     */
    size_t get_index(TileCoord coord) const {
        return static_cast<size_t>(coord.y) * width_ + static_cast<size_t>(coord.x);
    }

public:
    /**
     * @brief Allocates the elevation layer memory upfront.
     * @param width The horizontal size of the map in tiles.
     * @param height The vertical size of the map in tiles.
     */
    ElevationLayer(uint32_t width, uint32_t height)
        : width_(width), height_(height), elevation_data_(width * height, Fixed32(0)) {}

    /**
     * @brief Checks if a given coordinate is safely within the bounds of the map.
     */
    bool is_valid(TileCoord coord) const {
        return coord.x >= 0 && coord.x < static_cast<int32_t>(width_) &&
               coord.y >= 0 && coord.y < static_cast<int32_t>(height_);
    }

    /**
     * @brief Sets the elevation of a specific tile.
     * @param coord The target coordinate.
     * @param elevation The Fixed32 height value (typically -1.0 to 1.0).
     */
    void set_elevation(TileCoord coord, Fixed32 elevation) {
        ENGINE_ASSERT(is_valid(coord), "TileCoord out of bounds in ElevationLayer.");
        // Clamp to prevent catastrophic geological errors in generation
        elevation_data_[get_index(coord)] = MathUtils::clamp(elevation, Fixed32(-1.0f), Fixed32(1.0f));
    }

    /**
     * @brief Retrieves the elevation of a specific tile.
     * @param coord The target coordinate.
     * @return The Fixed32 height value.
     */
    Fixed32 get_elevation(TileCoord coord) const {
        ENGINE_ASSERT(is_valid(coord), "TileCoord out of bounds in ElevationLayer.");
        return elevation_data_[get_index(coord)];
    }

    /**
     * @brief Calculates the steepness (gradient) between two adjacent tiles.
     * @details Used by the Transport System to apply speed penalties on hills.
     * @param from The starting coordinate.
     * @param to The target coordinate.
     * @return The absolute elevation difference.
     */
    Fixed32 calculate_gradient(TileCoord from, TileCoord to) const {
        ENGINE_ASSERT(is_valid(from) && is_valid(to), "Gradient coordinates out of bounds.");
        Fixed32 diff = get_elevation(to) - get_elevation(from);
        // Absolute value calculation
        return diff < Fixed32(0) ? Fixed32(0) - diff : diff;
    }

    uint32_t get_width() const { return width_; }
    uint32_t get_height() const { return height_; }
};