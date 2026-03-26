#pragma once

#include <vector>
#include <cstdint>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/engine_assert.hpp"

/**
 * @file chunk_manager.hpp
 * @brief Manages spatial partitioning of the world to optimize simulation ticking and disk I/O.
 */

namespace ChunkConstants {
    /** @brief The standard dimension of a chunk in tiles. 16x16 is the AAA standard for macro-simulations. */
    constexpr int32_t CHUNK_SIZE = 16;
}

/**
 * @struct ChunkCoord
 * @brief A discrete coordinate representing a macro-region of the world.
 */
struct ChunkCoord {
    int32_t x = 0;
    int32_t y = 0;

    constexpr bool operator==(const ChunkCoord& other) const = default;
};

/**
 * @enum ChunkState
 * @brief Represents the current memory and simulation state of a chunk.
 */
enum class ChunkState : uint8_t {
    Unloaded = 0,   ///< Compressed on disk. Not in memory.
    Loaded,         ///< In memory for rendering, but simulation logic is suspended.
    Active,         ///< Fully loaded and actively ticking (entities moving, factories producing).
    Modified        ///< Flagged as dirty; must be serialized to disk before unloading.
};

/**
 * @class ChunkManager
 * @brief Tracks the state of all chunks and provides O(1) coordinate translation.
 */
class ChunkManager {
private:
    uint32_t map_width_tiles_;
    uint32_t map_height_tiles_;

    uint32_t chunk_cols_;
    uint32_t chunk_rows_;

    /** @brief A flat, contiguous array tracking the state of every chunk in the world. */
    std::vector<ChunkState> chunk_states_;

    /**
     * @brief Converts a 2D chunk coordinate into the 1D array index.
     */
    size_t get_index(ChunkCoord coord) const {
        return static_cast<size_t>(coord.y) * chunk_cols_ + static_cast<size_t>(coord.x);
    }

public:
    /**
     * @brief Initializes the chunk grid based on the total map dimensions.
     * @param map_width_tiles Total tiles across the X axis.
     * @param map_height_tiles Total tiles across the Y axis.
     */
    ChunkManager(uint32_t map_width_tiles, uint32_t map_height_tiles)
        : map_width_tiles_(map_width_tiles), map_height_tiles_(map_height_tiles) {

        // Calculate required chunks, rounding up if map size isn't perfectly divisible by 16
        chunk_cols_ = (map_width_tiles + ChunkConstants::CHUNK_SIZE - 1) / ChunkConstants::CHUNK_SIZE;
        chunk_rows_ = (map_height_tiles + ChunkConstants::CHUNK_SIZE - 1) / ChunkConstants::CHUNK_SIZE;

        chunk_states_.resize(chunk_cols_ * chunk_rows_, ChunkState::Unloaded);
    }

    /**
     * @brief Converts a world TileCoord into its corresponding ChunkCoord.
     * @param tile The specific tile coordinate.
     * @return The macro chunk coordinate.
     */
    constexpr ChunkCoord get_chunk_coord(TileCoord tile) const {
        return ChunkCoord{
            tile.x / ChunkConstants::CHUNK_SIZE,
            tile.y / ChunkConstants::CHUNK_SIZE
        };
    }

    /**
     * @brief Validates if a chunk coordinate exists within the world bounds.
     */
    bool is_valid(ChunkCoord coord) const {
        return coord.x >= 0 && coord.x < static_cast<int32_t>(chunk_cols_) &&
               coord.y >= 0 && coord.y < static_cast<int32_t>(chunk_rows_);
    }

    /**
     * @brief Updates the lifecycle state of a specific chunk.
     */
    void set_chunk_state(ChunkCoord coord, ChunkState state) {
        ENGINE_ASSERT(is_valid(coord), "ChunkCoord out of bounds in ChunkManager.");
        chunk_states_[get_index(coord)] = state;
    }

    /**
     * @brief Retrieves the current lifecycle state of a specific chunk.
     */
    ChunkState get_chunk_state(ChunkCoord coord) const {
        ENGINE_ASSERT(is_valid(coord), "ChunkCoord out of bounds in ChunkManager.");
        return chunk_states_[get_index(coord)];
    }

    uint32_t get_chunk_cols() const { return chunk_cols_; }
    uint32_t get_chunk_rows() const { return chunk_rows_; }
};