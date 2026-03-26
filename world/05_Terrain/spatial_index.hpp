#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <span>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/entity_id.hpp"
#include "../../core/04_Types/engine_assert.hpp"

/**
 * @file spatial_index.hpp
 * @brief Maintains a high-speed, per-tile ledger of active entities.
 * @details Isolated into its own SoA layer to maximize CPU cache efficiency during collision, logistics, and AI targeting queries.
 */

/**
 * @class SpatialIndex
 * @brief A flattened 1D array of entity lists, mapping coordinates directly to the entities occupying them.
 */
class SpatialIndex {
private:
    uint32_t width_;
    uint32_t height_;

    /** * @brief The flat array of entity vectors.
     * @details While vector-of-vectors isn't perfectly contiguous, it is the most stable AAA approach
     * for dynamic grids where a tile might hold 0 entities or 50 entities at any given moment.
     */
    std::vector<std::vector<EntityID>> spatial_grid_;

    /**
     * @brief Converts a 2D coordinate into the 1D array index.
     */
    size_t get_index(TileCoord coord) const {
        return static_cast<size_t>(coord.y) * width_ + static_cast<size_t>(coord.x);
    }

public:
    /**
     * @brief Allocates the spatial grid memory upfront.
     * @param width The horizontal size of the map in tiles.
     * @param height The vertical size of the map in tiles.
     */
    SpatialIndex(uint32_t width, uint32_t height)
        : width_(width), height_(height), spatial_grid_(width * height) {}

    /**
     * @brief Checks if a given coordinate is safely within the bounds of the map.
     */
    bool is_valid(TileCoord coord) const {
        return coord.x >= 0 && coord.x < static_cast<int32_t>(width_) &&
               coord.y >= 0 && coord.y < static_cast<int32_t>(height_);
    }

    /**
     * @brief Registers an entity's presence on a specific tile.
     * @param coord The target coordinate.
     * @param entity The EntityID to register.
     */
    void add_entity(TileCoord coord, EntityID entity) {
        ENGINE_ASSERT(is_valid(coord), "TileCoord out of bounds in SpatialIndex.");
        spatial_grid_[get_index(coord)].push_back(entity);
    }

    /**
     * @brief Removes an entity from a tile using an ultra-fast O(1) swap-and-pop.
     * @param coord The target coordinate.
     * @param entity The EntityID to remove.
     */
    void remove_entity(TileCoord coord, EntityID entity) {
        ENGINE_ASSERT(is_valid(coord), "TileCoord out of bounds in SpatialIndex.");
        auto& cell_entities = spatial_grid_[get_index(coord)];

        auto it = std::find(cell_entities.begin(), cell_entities.end(), entity);
        if (it != cell_entities.end()) {
            *it = cell_entities.back(); // Overwrite the target with the last element
            cell_entities.pop_back();   // Destroy the duplicated last element
        }
    }

    /**
     * @brief Safely updates an entity's position from one tile to another.
     * @param old_coord The tile the entity is leaving.
     * @param new_coord The tile the entity is entering.
     * @param entity The moving EntityID.
     */
    void move_entity(TileCoord old_coord, TileCoord new_coord, EntityID entity) {
        if (old_coord == new_coord) return; // Ignore zero-distance moves
        remove_entity(old_coord, entity);
        add_entity(new_coord, entity);
    }

    /**
     * @brief Retrieves a read-only view of all entities on a specific tile.
     * @param coord The target coordinate.
     * @return A std::span providing a safe, zero-allocation view of the internal vector.
     */
    std::span<const EntityID> get_entities_at(TileCoord coord) const {
        ENGINE_ASSERT(is_valid(coord), "TileCoord out of bounds in SpatialIndex.");
        return spatial_grid_[get_index(coord)];
    }
};