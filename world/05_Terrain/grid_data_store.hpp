#pragma once

#include <vector>
#include <algorithm>
#include <cstdint>

#include "../core/04_Types/tile_coord.hpp"
#include "../core/04_Types/terrain_type.hpp"
#include "../core/04_Types/fixed_point.hpp"
#include "../core/04_Types/entity_id.hpp"
#include "../core/04_Types/engine_assert.hpp"

/**
 * @file grid_data_store.hpp
 * @brief Defines the spatial backbone of the world map using a cache-friendly flattened 1D array.
 */

/**
 * @struct GridCell
 * @brief Represents a single discrete location on the world map.
 * @details Stores physical properties and a dynamic list of entities currently occupying the tile.
 */
struct GridCell {
    /** @brief The geographical classification of the tile. */
    TerrainType terrain = TerrainType::Plains;

    /** @brief The topological height of the tile. */
    Fixed32 elevation{0};

    /** @brief The water saturation level of the tile. */
    Fixed32 moisture{0};

    /** @brief A list of all entities (buildings, vehicles, citizens) occupying this coordinate. */
    std::vector<EntityID> entities;
};

/**
 * @class GridDataStore
 * @brief Manages the contiguous memory block representing the entire simulation grid.
 */
class GridDataStore {
private:
    uint32_t width_;
    uint32_t height_;
    std::vector<GridCell> cells_;

    /**
     * @brief Converts a 2D coordinate into a 1D memory index.
     * @param coord The target grid coordinate.
     * @return The flattened array index.
     */
    size_t get_index(TileCoord coord) const {
        return static_cast<size_t>(coord.y) * width_ + static_cast<size_t>(coord.x);
    }

public:
    /**
     * @brief Allocates the world grid memory upfront.
     * @param width The horizontal size of the map in tiles.
     * @param height The vertical size of the map in tiles.
     */
    GridDataStore(uint32_t width, uint32_t height)
        : width_(width), height_(height), cells_(width * height) {}

    /**
     * @brief Checks if a given coordinate is safely within the bounds of the map.
     * @param coord The coordinate to evaluate.
     * @return True if the coordinate is within the allocated grid.
     */
    bool is_valid(TileCoord coord) const {
        return coord.x >= 0 && coord.x < static_cast<int32_t>(width_) &&
               coord.y >= 0 && coord.y < static_cast<int32_t>(height_);
    }

    /**
     * @brief Retrieves a mutable reference to a specific grid cell.
     * @param coord The target coordinate.
     * @return A reference to the GridCell.
     */
    GridCell& get_cell(TileCoord coord) {
        ENGINE_ASSERT(is_valid(coord), "TileCoord out of bounds in GridDataStore.");
        return cells_[get_index(coord)];
    }

    /**
     * @brief Retrieves a read-only reference to a specific grid cell.
     * @param coord The target coordinate.
     * @return A const reference to the GridCell.
     */
    const GridCell& get_cell(TileCoord coord) const {
        ENGINE_ASSERT(is_valid(coord), "TileCoord out of bounds in GridDataStore.");
        return cells_[get_index(coord)];
    }

    /**
     * @brief Registers an entity's presence on a specific tile.
     * @param coord The target coordinate.
     * @param entity The EntityID to register.
     */
    void add_entity(TileCoord coord, EntityID entity) {
        get_cell(coord).entities.push_back(entity);
    }

    /**
     * @brief Removes an entity from a tile using an O(1) swap-and-pop operation.
     * @param coord The target coordinate.
     * @param entity The EntityID to remove.
     */
    void remove_entity(TileCoord coord, EntityID entity) {
        auto& ents = get_cell(coord).entities;
        auto it = std::find(ents.begin(), ents.end(), entity);
        if (it != ents.end()) {
            *it = ents.back(); // Overwrite the target with the last element
            ents.pop_back();   // Remove the duplicated last element
        }
    }

    /** @brief Retrieves the grid width. */
    uint32_t get_width() const { return width_; }

    /** @brief Retrieves the grid height. */
    uint32_t get_height() const { return height_; }
};