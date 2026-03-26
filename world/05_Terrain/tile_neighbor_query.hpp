#pragma once

#include <array>
#include <cstdint>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/direction.hpp"

/**
 * @file tile_neighbor_query.hpp
 * @brief High-performance, zero-allocation neighbor querying for grid coordinates.
 * @details Utilized by pathfinding, diffusion, and adjacency checks to safely query adjacent tiles without heap allocation overhead.
 */

namespace TileNeighborQuery {

    /**
     * @struct NeighborList
     * @brief A stack-allocated container for adjacent coordinates.
     * @tparam MaxSize The maximum number of neighbors (typically 4 or 8).
     */
    template <size_t MaxSize>
    struct NeighborList {
        /** @brief The raw stack array of coordinates. */
        std::array<TileCoord, MaxSize> coordinates{};

        /** @brief The number of valid coordinates currently stored in the array. */
        uint8_t count = 0;

        /**
         * @brief Appends a coordinate to the list.
         * @param coord The valid neighbor coordinate.
         */
        constexpr void push_back(TileCoord coord) {
            if (count < MaxSize) {
                coordinates[count++] = coord;
            }
        }
    };

    /**
     * @brief Internal helper to verify if a coordinate is strictly within map bounds.
     */
    constexpr bool is_valid(TileCoord coord, int32_t width, int32_t height) {
        return coord.x >= 0 && coord.x < width && coord.y >= 0 && coord.y < height;
    }

    /**
     * @brief Retrieves up to 4 valid orthogonal (North, East, South, West) neighbors.
     * @param center The origin coordinate.
     * @param width The map width.
     * @param height The map height.
     * @return A stack-allocated NeighborList containing 2 to 4 valid coordinates.
     */
    constexpr NeighborList<4> get_cardinals(TileCoord center, int32_t width, int32_t height) {
        NeighborList<4> list;
        for (Direction dir : DirectionUtils::CARDINALS) {
            auto offset = DirectionUtils::OFFSETS[static_cast<size_t>(dir)];
            TileCoord neighbor{center.x + offset.dx, center.y + offset.dy};

            if (is_valid(neighbor, width, height)) {
                list.push_back(neighbor);
            }
        }
        return list;
    }

    /**
     * @brief Retrieves up to 8 valid orthogonal and diagonal neighbors.
     * @param center The origin coordinate.
     * @param width The map width.
     * @param height The map height.
     * @return A stack-allocated NeighborList containing 3 to 8 valid coordinates.
     */
    constexpr NeighborList<8> get_all(TileCoord center, int32_t width, int32_t height) {
        NeighborList<8> list;
        for (size_t i = 0; i < 8; ++i) {
            auto offset = DirectionUtils::OFFSETS[i];
            TileCoord neighbor{center.x + offset.dx, center.y + offset.dy};

            if (is_valid(neighbor, width, height)) {
                list.push_back(neighbor);
            }
        }
        return list;
    }
}