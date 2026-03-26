#pragma once

#include <cstdint>
#include <array>

/**
 * @file direction.hpp
 * @brief Defines cardinal and diagonal directions with associated grid offset lookup tables.
 * @details Used extensively by pathfinding, cellular automata, and neighbor-querying systems.
 * Assumes a screen-space coordinate system (+x is East, +y is South) compatible with Godot 4.
 */

/**
 * @enum Direction
 * @brief An 8-way directional compass.
 * @details Strictly ordered clockwise starting from North to enable fast modular arithmetic.
 */
enum class Direction : uint8_t {
    North = 0,
    NorthEast,
    East,
    SouthEast,
    South,
    SouthWest,
    West,
    NorthWest,

    /** @brief The total number of valid directions. */
    COUNT,

    /** @brief Represents a null or invalid direction. */
    None = 255
};

/**
 * @struct DirectionOffset
 * @brief Represents a discrete 2D grid step.
 */
struct DirectionOffset {
    int32_t dx;
    int32_t dy;
};

/**
 * @namespace DirectionUtils
 * @brief Compile-time lookup tables and fast-math helpers for directional queries.
 */
namespace DirectionUtils {

    /**
     * @brief Maps each Direction enum value to its corresponding (dx, dy) grid offset.
     * @details Indexed directly by the enum value. Zero-cost lookup at runtime.
     */
    constexpr std::array<DirectionOffset, static_cast<size_t>(Direction::COUNT)> OFFSETS = {{
        { 0, -1}, // North
        { 1, -1}, // NorthEast
        { 1,  0}, // East
        { 1,  1}, // SouthEast
        { 0,  1}, // South
        {-1,  1}, // SouthWest
        {-1,  0}, // West
        {-1, -1}  // NorthWest
    }};

    /**
     * @brief Convenience array containing only the 4 cardinal directions.
     * @details Useful for systems that restrict diagonal movement (e.g., pipes, simple roads).
     */
    constexpr std::array<Direction, 4> CARDINALS = {
        Direction::North, Direction::East, Direction::South, Direction::West
    };

    /**
     * @brief Calculates the exact opposite direction using zero-branching modular arithmetic.
     * @param dir The input direction.
     * @return The 180-degree inverted direction. Returns None if the input is None.
     */
    constexpr Direction getOpposite(Direction dir) {
        if (dir == Direction::None || dir == Direction::COUNT) return Direction::None;
        // Add 4 to flip 180 degrees on an 8-way compass, modulo 8 wraps it safely.
        return static_cast<Direction>((static_cast<uint8_t>(dir) + 4) % 8);
    }
}