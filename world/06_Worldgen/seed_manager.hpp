#pragma once

#include <cstdint>
#include <random>
#include <string>
#include <functional>

#include "../../core/04_Types/fixed_point.hpp"

/**
 * @file seed_manager.hpp
 * @brief Manages 64-bit deterministic randomness for procedural world generation.
 * @details Wraps the standard 64-bit Mersenne Twister engine to guarantee map reproduction across platforms.
 */

class SeedManager {
private:
    /** @brief The master 64-bit integer that defines the entire world state. */
    uint64_t current_seed_;

    /** @brief High-quality, 64-bit procedural random number engine. */
    std::mt19937_64 rng_engine_;

public:
    /**
     * @brief Entropy Constructor: Generates a truly random 64-bit seed using OS hardware entropy.
     */
    SeedManager() {
        std::random_device rd;
        // std::random_device typically returns 32 bits. We shift and combine two calls to guarantee a 64-bit seed.
        current_seed_ = (static_cast<uint64_t>(rd()) << 32) | rd();
        rng_engine_.seed(current_seed_);
    }

    /**
     * @brief Explicit Integer Constructor: Locks the RNG to a known sequence.
     * @param seed The specific 64-bit world seed.
     */
    explicit SeedManager(uint64_t seed) : current_seed_(seed), rng_engine_(seed) {}

    /**
     * @brief String Constructor: Allows players to type words like "Systemcraft" to generate a map.
     * @param string_seed A human-readable text seed.
     */
    explicit SeedManager(const std::string& string_seed) {
        // Hashes the string into a deterministic 64-bit integer
        current_seed_ = std::hash<std::string>{}(string_seed);
        rng_engine_.seed(current_seed_);
    }

    /**
     * @brief Retrieves the current seed (used for save files and UI display).
     */
    uint64_t get_seed() const { return current_seed_; }

    /**
     * @brief Generates a random integer within an inclusive range.
     */
    int32_t random_int(int32_t min, int32_t max) {
        std::uniform_int_distribution<int32_t> dist(min, max);
        return dist(rng_engine_);
    }

    /**
     * @brief Generates a random Fixed32 floating-point number within an inclusive range.
     * @details Primarily used by noise algorithms to generate values between -1.0 and 1.0.
     */
    Fixed32 random_fixed(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return Fixed32(dist(rng_engine_));
    }
};