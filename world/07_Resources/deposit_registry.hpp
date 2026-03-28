#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/entity_id.hpp"

/**
 * @file deposit_registry.hpp
 * @brief A spatial index enabling O(1) lookups for resource deposits on the world map.
 */

namespace Resources {

    /**
     * @class DepositRegistry
     * @brief Maps TileCoords to a list of ResourceDeposit entities physically present there.
     */
    class DepositRegistry {
    private:
        uint32_t width_;
        uint32_t height_;

        // A flat 1D array representing the 2D grid. Each cell holds a dynamic list of EntityIDs.
        // We use a flat array to maximize CPU cache coherency during spatial lookups.
        std::vector<std::vector<EntityID>> grid_;

        /**
         * @brief Converts a 2D coordinate into a flat 1D memory index.
         */
        inline size_t get_index(TileCoord coord) const {
            return static_cast<size_t>(coord.y) * width_ + static_cast<size_t>(coord.x);
        }

        /**
         * @brief Validates that a coordinate actually exists on the map.
         */
        inline bool is_valid(TileCoord coord) const {
            return coord.x >= 0 && coord.x < static_cast<int32_t>(width_) &&
                   coord.y >= 0 && coord.y < static_cast<int32_t>(height_);
        }

    public:
        /**
         * @brief Constructs a global index sized to the world map.
         * @param w The width of the world.
         * @param h The height of the world.
         */
        DepositRegistry(uint32_t w, uint32_t h) : width_(w), height_(h) {
            grid_.resize(static_cast<size_t>(w) * h);
        }

        /**
         * @brief Registers a newly spawned resource entity at a specific map coordinate.
         * @param coord The physical location on the map.
         * @param deposit_entity The exact ECS EntityID of the deposit.
         */
        void register_deposit(TileCoord coord, EntityID deposit_entity) {
            if (is_valid(coord)) {
                grid_[get_index(coord)].push_back(deposit_entity);
            }
        }

        /**
         * @brief Retrieves all resource deposit entities present on a specific tile.
         * @param coord The map coordinate to query.
         * @return A constant reference to the vector of EntityIDs.
         */
        const std::vector<EntityID>& get_deposits(TileCoord coord) const {
            // A safe, empty fallback to prevent vector allocation overhead if queried out-of-bounds
            static const std::vector<EntityID> empty_fallback;

            if (is_valid(coord)) {
                return grid_[get_index(coord)];
            }
            return empty_fallback;
        }

        /**
         * @brief Removes a specific deposit entity from a tile (e.g., when fully exhausted).
         * @param coord The map coordinate.
         * @param deposit_entity The exact ECS EntityID to unregister.
         */
        void remove_deposit(TileCoord coord, EntityID deposit_entity) {
            if (is_valid(coord)) {
                auto& cell = grid_[get_index(coord)];

                // Erase-remove idiom targeting the specific raw ID
                cell.erase(std::remove_if(cell.begin(), cell.end(),
                    [&](const EntityID& e) { return e.raw_id == deposit_entity.raw_id; }),
                    cell.end());
            }
        }

        /**
         * @brief Clears all registrations from the index (used during teardown/reloads).
         */
        void clear() {
            for (auto& cell : grid_) {
                cell.clear();
            }
        }
    };
}