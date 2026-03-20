#pragma once

#include <cstdint>

/**
 * @file tick_counter.hpp
 * @brief Maintains the master simulation tick for the engine.
 */

/**
 * @class TickCounter
 * @brief A monotonically increasing 64-bit integer tracking the current engine state.
 * * Used to timestamp events, synchronize multiplayer network states, and ensure
 * deterministic gameplay. Every fixed physics step increments this counter by 1.
 */
class TickCounter {
private:
    /**
     * @var current_tick
     * @brief The absolute current tick of the simulation.
     */
    uint64_t current_tick;

public:
    /**
     * @brief Initializes a brand new simulation starting at tick zero.
     */
    TickCounter() : current_tick(0) {}

    /**
     * @brief Initializes the simulation at a specific tick.
     * * Critical for loading saved games or syncing a client to a multiplayer server.
     * @param starting_tick The tick to resume from.
     */
    TickCounter(uint64_t starting_tick) : current_tick(starting_tick) {}

    /**
     * @brief Increments the global simulation tick by exactly one.
     * * MUST be called exactly once at the end of every fixed update loop.
     */
    void increment() {
        current_tick++;
    }

    /**
     * @brief Retrieves the current simulation tick.
     * @return uint64_t The exact 64-bit timestamp.
     */
    uint64_t get() const {
        return current_tick;
    }
};