#pragma once

#include <cstdint>
#include <cmath>
#include <vector>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/fixed_point.hpp"
#include "../../core/04_Types/math_utils.hpp"
#include "seed_manager.hpp"

/**
 * @file heightmap_generator.hpp
 * @brief Generates organic, fractal elevation data using Fractional Brownian Motion (fBm).
 */

/**
 * @struct NoiseParameters
 * @brief Tunable settings for the fractal noise generation.
 */
struct NoiseParameters {
    /** @brief The base zoom level of the map. Larger = smoother, wider hills. */
    float scale = 50.0f;

    /** @brief How many layers of noise to stack. (1 = smooth, 6 = highly detailed/craggy). */
    int32_t octaves = 4;

    /** @brief How much each successive octave contributes to the overall height (usually 0.5). */
    float persistence = 0.5f;

    /** @brief How much the frequency increases per octave (usually 2.0). */
    float lacunarity = 2.0f;
};

namespace HeightmapGenerator {

    /**
     * @brief A stateless, pseudo-random hash function for 2D coordinates.
     * @details Replaces the need for a massive, stateful permutation table.
     */
    inline float random_gradient(int32_t ix, int32_t iy, uint64_t seed) {
        // Simple bitwise hash optimized for grid coordinates
        uint64_t hash = static_cast<uint64_t>(ix) * 374761393ULL + static_cast<uint64_t>(iy) * 668265263ULL + seed;
        hash = (hash ^ (hash >> 13)) * 1274126177ULL;

        // Return a float between -1.0 and 1.0
        return static_cast<float>((hash & 0x0FFFFFFF)) / static_cast<float>(0x0FFFFFFF / 2) - 1.0f;
    }

    /**
     * @brief Smoothstep fade function to round off the edges of the grid cells.
     */
    constexpr float fade(float t) {
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }

    /**
     * @brief Generates a single layer of smooth Value Noise.
     */
    inline float single_layer_noise(float x, float y, uint64_t seed) {
        // Find grid cell coordinates
        int32_t x0 = static_cast<int32_t>(std::floor(x));
        int32_t y0 = static_cast<int32_t>(std::floor(y));
        int32_t x1 = x0 + 1;
        int32_t y1 = y0 + 1;

        // Determine interpolation weights
        float sx = fade(x - static_cast<float>(x0));
        float sy = fade(y - static_cast<float>(y0));

        // Sample the 4 corners of the grid cell
        float n0 = random_gradient(x0, y0, seed);
        float n1 = random_gradient(x1, y0, seed);
        float n2 = random_gradient(x0, y1, seed);
        float n3 = random_gradient(x1, y1, seed);

        // Bilinear interpolation
        float ix0 = n0 + sx * (n1 - n0);
        float ix1 = n2 + sx * (n3 - n2);
        return ix0 + sy * (ix1 - ix0);
    }

    /**
     * @brief Generates fractal elevation for a specific tile coordinate.
     * @param coord The target grid location.
     * @param params The tuning constraints for the terrain.
     * @param seed_manager The global world RNG.
     * @return A Fixed32 elevation safely clamped between -1.0 and 1.0.
     */
    inline Fixed32 generate_elevation(TileCoord coord, const NoiseParameters& params, SeedManager& seed_manager) {
        float amplitude = 1.0f;
        float frequency = 1.0f;
        float noise_height = 0.0f;
        float max_possible_height = 0.0f; // Used to normalize the final result

        // Base seed for this specific coordinate evaluation
        uint64_t base_seed = seed_manager.get_seed();

        for (int32_t i = 0; i < params.octaves; ++i) {
            // Calculate spatial frequency
            float sample_x = (static_cast<float>(coord.x) / params.scale) * frequency;
            float sample_y = (static_cast<float>(coord.y) / params.scale) * frequency;

            // Offset the seed per octave to prevent visible echoing/stacking artifacts
            uint64_t octave_seed = base_seed + static_cast<uint64_t>(i * 99991);

            float noise_val = single_layer_noise(sample_x, sample_y, octave_seed);

            noise_height += noise_val * amplitude;
            max_possible_height += amplitude;

            // Prepare values for the next detailed layer
            amplitude *= params.persistence;
            frequency *= params.lacunarity;
        }

        // Normalize back to roughly -1.0 to 1.0
        float normalized = noise_height / max_possible_height;

        // Clamp it safely into our Fixed32 engine limits
        return MathUtils::clamp(Fixed32(normalized), Fixed32(-1.0f), Fixed32(1.0f));
    }
}