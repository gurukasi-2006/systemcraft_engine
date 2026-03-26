#pragma once

#include <vector>
#include <cstdint>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/fixed_point.hpp"
#include "../../core/04_Types/engine_assert.hpp"
#include "../../core/04_Types/direction.hpp"
#include "elevation_layer.hpp"

/**
 * @file water_body_layer.hpp
 * @brief Manages the hydrological mapping of rivers, lakes, and oceans.
 * @details Uses steepest-descent pathfinding to trace rivers from high elevations down to sea level.
 */

/**
 * @enum WaterBodyType
 * @brief Classifies the specific hydrological state of a tile.
 */
enum class WaterBodyType : uint8_t {
    None = 0,
    River,
    Lake,
    Ocean
};

/**
 * @class WaterBodyLayer
 * @brief A contiguous 1D array of water classifications and river widths.
 */
class WaterBodyLayer {
private:
    uint32_t width_;
    uint32_t height_;

    /** @brief The SoA array classifying the water type of each tile. */
    std::vector<WaterBodyType> types_;

    /** @brief The SoA array recording the width/volume of the river for transport mechanics. */
    std::vector<Fixed32> widths_;

    /**
     * @brief Converts a 2D coordinate into the 1D array index.
     */
    size_t get_index(TileCoord coord) const {
        return static_cast<size_t>(coord.y) * width_ + static_cast<size_t>(coord.x);
    }

public:
    /**
     * @brief Allocates the water body arrays upfront.
     * @param width The horizontal size of the map in tiles.
     * @param height The vertical size of the map in tiles.
     */
    WaterBodyLayer(uint32_t width, uint32_t height)
        : width_(width), height_(height),
          types_(width * height, WaterBodyType::None),
          widths_(width * height, Fixed32(0)) {}

    /**
     * @brief Checks if a given coordinate is safely within the bounds of the map.
     */
    bool is_valid(TileCoord coord) const {
        return coord.x >= 0 && coord.x < static_cast<int32_t>(width_) &&
               coord.y >= 0 && coord.y < static_cast<int32_t>(height_);
    }

    /**
     * @brief Overrides the water type of a specific tile.
     */
    void set_type(TileCoord coord, WaterBodyType type) {
        ENGINE_ASSERT(is_valid(coord), "TileCoord out of bounds in WaterBodyLayer.");
        types_[get_index(coord)] = type;
    }

    /**
     * @brief Retrieves the water type of a specific tile.
     */
    WaterBodyType get_type(TileCoord coord) const {
        ENGINE_ASSERT(is_valid(coord), "TileCoord out of bounds in WaterBodyLayer.");
        return types_[get_index(coord)];
    }

    /**
     * @brief Sets the width/volume of the river at a specific tile.
     */
    void set_width(TileCoord coord, Fixed32 width) {
        ENGINE_ASSERT(is_valid(coord), "TileCoord out of bounds in WaterBodyLayer.");
        widths_[get_index(coord)] = width;
    }

    /**
     * @brief Retrieves the river width at a specific tile.
     */
    Fixed32 get_width(TileCoord coord) const {
        ENGINE_ASSERT(is_valid(coord), "TileCoord out of bounds in WaterBodyLayer.");
        return widths_[get_index(coord)];
    }

    /**
     * @brief Traces a river from a source coordinate downhill until it hits a lake or the ocean.
     * @param start The origin spring coordinate.
     * @param elevation_layer A read-only reference to the world's elevation data.
     * @param starting_width The initial width of the stream.
     * @param width_growth The amount of width/volume gained per tile of travel.
     */
    void trace_river(TileCoord start, const ElevationLayer& elevation_layer, Fixed32 starting_width, Fixed32 width_growth) {
        TileCoord current = start;
        Fixed32 current_width = starting_width;

        while (true) {
            if (!is_valid(current)) break;

            // If we merge into an existing lake or ocean, the river naturally ends
            WaterBodyType current_type = get_type(current);
            if (current_type == WaterBodyType::Ocean || current_type == WaterBodyType::Lake) {
                break;
            }

            // Mark current tile as River and update width (taking the max if rivers converge)
            set_type(current, WaterBodyType::River);
            Fixed32 existing_width = get_width(current);
            set_width(current, existing_width > current_width ? existing_width : current_width);

            Fixed32 current_elev = elevation_layer.get_elevation(current);

            // Sea-level termination
            if (current_elev <= Fixed32(0.0f)) {
                set_type(current, WaterBodyType::Ocean);
                break;
            }

            // --- STEEPEST DESCENT SEARCH ---
            TileCoord next_coord = current;
            Fixed32 lowest_elev = current_elev;

            // Check all 8 neighboring tiles (0 to 7 mapping to DirectionUtils::OFFSETS)
            for (uint8_t i = 0; i < 8; ++i) {
                auto offset = DirectionUtils::OFFSETS[i];
                TileCoord neighbor{current.x + offset.dx, current.y + offset.dy};

                if (elevation_layer.is_valid(neighbor)) {
                    Fixed32 neighbor_elev = elevation_layer.get_elevation(neighbor);
                    // Must be strictly lower to guarantee gravity flow and prevent infinite flat-loops
                    if (neighbor_elev < lowest_elev) {
                        lowest_elev = neighbor_elev;
                        next_coord = neighbor;
                    }
                }
            }

            // LOCAL MINIMUM: If no neighbor is lower, the river has nowhere to drain. Form a lake.
            if (next_coord.x == current.x && next_coord.y == current.y) {
                set_type(current, WaterBodyType::Lake);
                break;
            }

            // Advance the tracer downhill
            current = next_coord;
            current_width = current_width + width_growth;
        }
    }
};