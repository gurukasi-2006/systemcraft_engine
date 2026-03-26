#pragma once

#include <cstdint>
#include <functional>

/**
 * @file tile_coord.hpp
 * @brief Defines the canonical three-dimensional coordinate system for the grid-based simulation world.
 */

/**
 * @struct TileCoord
 * @brief Represents a discrete spatial location within the engine's chunk and rendering systems.
 */
struct TileCoord {
    /**
     * @brief The horizontal X-axis position on the simulation grid.
     */
    int32_t x = 0;

    /**
     * @brief The vertical Y-axis position on the simulation grid.
     */
    int32_t y = 0;

    /**
     * @brief The depth or elevation level. 0 represents the surface layer.
     * Positive values indicate elevation, negative values indicate underground layers.
     */
    int32_t layer = 0;

    /**
     * @brief Constructs a TileCoord initialized to the origin (0, 0, 0).
     */
    TileCoord() = default;

    /**
     * @brief Constructs a TileCoord with specific spatial coordinates.
     * @param x_val The targeted X coordinate.
     * @param y_val The targeted Y coordinate.
     * @param layer_val The targeted layer/elevation (defaults to 0).
     */
    TileCoord(int32_t x_val, int32_t y_val, int32_t layer_val = 0)
        : x(x_val), y(y_val), layer(layer_val) {}

    /**
     * @brief Evaluates strict spatial equality between two coordinates.
     * @param other The coordinate to compare against.
     * @return True if x, y, and layer are exactly identical.
     */
    bool operator==(const TileCoord& other) const {
        return x == other.x && y == other.y && layer == other.layer;
    }

    /**
     * @brief Evaluates spatial inequality between two coordinates.
     * @param other The coordinate to compare against.
     * @return True if any axis or layer differs.
     */
    bool operator!=(const TileCoord& other) const {
        return !(*this == other);
    }

    /**
     * @brief Adds a directional vector or offset to the current coordinate.
     * @param other The coordinate offset to add.
     * @return A new TileCoord representing the translated position.
     */
    TileCoord operator+(const TileCoord& other) const {
        return TileCoord(x + other.x, y + other.y, layer + other.layer);
    }

    /**
     * @brief Subtracts a directional vector or offset from the current coordinate.
     * @param other The coordinate offset to subtract.
     * @return A new TileCoord representing the translated position.
     */
    TileCoord operator-(const TileCoord& other) const {
        return TileCoord(x - other.x, y - other.y, layer - other.layer);
    }
};

/**
 * @namespace std
 * @brief Standard library injections for Systemcraft engine types.
 */
namespace std {
    /**
     * @struct hash<TileCoord>
     * @brief Provides a deterministic hashing algorithm for TileCoord, allowing its use as an unordered map key.
     */
    template <>
    struct hash<TileCoord> {
        /**
         * @brief Computes the hash value using a bitwise combination of the x, y, and layer properties.
         * @param coord The coordinate to hash.
         * @return A 64-bit size_t hash representation.
         */
        std::size_t operator()(const TileCoord& coord) const {
            std::size_t h1 = std::hash<int32_t>{}(coord.x);
            std::size_t h2 = std::hash<int32_t>{}(coord.y);
            std::size_t h3 = std::hash<int32_t>{}(coord.layer);

            std::size_t hash = h1;
            hash ^= h2 + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            hash ^= h3 + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            return hash;
        }
    };
}