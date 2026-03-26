#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/fixed_point.hpp"
#include "../../core/04_Types/engine_assert.hpp"
#include "../../core/04_Types/math_utils.hpp"
#include "../../core/04_Types/direction.hpp"

/**
 * @file moisture_layer.hpp
 * @brief Manages the hydrological saturation map of the simulation.
 * @details Utilizes a double-buffered Structure of Arrays (SoA) to perform order-independent deterministic diffusion passes.
 */

/**
 * @class MoistureLayer
 * @brief A contiguous 1D array of fixed-point moisture values representing fertility and water saturation.
 */
class MoistureLayer {
private:
    uint32_t width_;
    uint32_t height_;

    /** @brief The primary array of moisture values (0.0 to 1.0). */
    std::vector<Fixed32> moisture_data_;

    /** @brief A secondary buffer used exclusively during the diffusion pass to prevent directional bias. */
    std::vector<Fixed32> back_buffer_;

    /**
     * @brief Converts a 2D coordinate into the 1D array index.
     */
    size_t get_index(TileCoord coord) const {
        return static_cast<size_t>(coord.y) * width_ + static_cast<size_t>(coord.x);
    }

public:
    /**
     * @brief Allocates the primary and back-buffer memory upfront.
     * @param width The horizontal size of the map in tiles.
     * @param height The vertical size of the map in tiles.
     */
    MoistureLayer(uint32_t width, uint32_t height)
        : width_(width), height_(height),
          moisture_data_(width * height, Fixed32(0)),
          back_buffer_(width * height, Fixed32(0)) {}

    /**
     * @brief Checks if a given coordinate is safely within the bounds of the map.
     */
    bool is_valid(TileCoord coord) const {
        return coord.x >= 0 && coord.x < static_cast<int32_t>(width_) &&
               coord.y >= 0 && coord.y < static_cast<int32_t>(height_);
    }

    /**
     * @brief Sets the absolute moisture of a specific tile.
     * @param coord The target coordinate.
     * @param moisture The Fixed32 moisture value (clamped 0.0 to 1.0).
     */
    void set_moisture(TileCoord coord, Fixed32 moisture) {
        ENGINE_ASSERT(is_valid(coord), "TileCoord out of bounds in MoistureLayer.");
        moisture_data_[get_index(coord)] = MathUtils::clamp(moisture, Fixed32(0.0f), Fixed32(1.0f));
    }

    /**
     * @brief Retrieves the moisture of a specific tile.
     * @param coord The target coordinate.
     * @return The Fixed32 moisture value.
     */
    Fixed32 get_moisture(TileCoord coord) const {
        ENGINE_ASSERT(is_valid(coord), "TileCoord out of bounds in MoistureLayer.");
        return moisture_data_[get_index(coord)];
    }

    /**
     * @brief Executes a single deterministic diffusion pass across the entire grid.
     * @details Simulates water seepage from rivers and coasts into adjacent tiles.
     * Uses a double-buffer to guarantee that the evaluation order does not bias the spread direction.
     * @param diffusion_rate The fraction of the difference to equalize per pass (e.g., 0.1).
     */
    void diffuse(Fixed32 diffusion_rate) {
        // Clamp the rate to prevent mathematical instability
        Fixed32 safe_rate = MathUtils::clamp(diffusion_rate, Fixed32(0.0f), Fixed32(1.0f));

        for (int32_t y = 0; y < static_cast<int32_t>(height_); ++y) {
            for (int32_t x = 0; x < static_cast<int32_t>(width_); ++x) {
                TileCoord current{x, y};
                Fixed32 current_val = get_moisture(current);
                Fixed32 neighbor_sum(0);
                Fixed32 valid_neighbors(0);

                // Sample the 4 cardinal neighbors
                for (Direction dir : DirectionUtils::CARDINALS) {
                    auto offset = DirectionUtils::OFFSETS[static_cast<size_t>(dir)];
                    TileCoord neighbor{x + offset.dx, y + offset.dy};

                    if (is_valid(neighbor)) {
                        neighbor_sum = neighbor_sum + get_moisture(neighbor);
                        valid_neighbors = valid_neighbors + Fixed32(1);
                    }
                }

                // If surrounded by valid tiles, calculate the average and blend
                if (valid_neighbors > Fixed32(0)) {
                    Fixed32 neighbor_avg = neighbor_sum / valid_neighbors;
                    Fixed32 difference = neighbor_avg - current_val;
                    Fixed32 diffused_val = current_val + (difference * safe_rate);

                    // Write strictly to the back buffer
                    back_buffer_[get_index(current)] = MathUtils::clamp(diffused_val, Fixed32(0.0f), Fixed32(1.0f));
                } else {
                    back_buffer_[get_index(current)] = current_val;
                }
            }
        }

        // Swap buffers to commit the state simultaneously
        moisture_data_.swap(back_buffer_);
    }

    uint32_t get_width() const { return width_; }
    uint32_t get_height() const { return height_; }
};