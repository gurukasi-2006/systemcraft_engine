#pragma once

#include <chrono>
#include<thread>

/**
 * @file delta_time_calculator.hpp
 * @brief Computes the real-world elapsed time between engine frames.
 */

/**
 * @class DeltaTimeCalculator
 * @brief A high-precision stopwatch for decoupling simulation speed from frame rate.
 * * Measures 'Delta Time' (dt) in seconds. Includes built-in safety limits to prevent
 * the engine from calculating massive time jumps if the OS stutters or the window is dragged.
 */
class DeltaTimeCalculator {
private:
    // Typedefs to make chrono's verbose types manageable
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    /**
     * @brief The exact CPU time recorded during the previous frame.
     */
    TimePoint last_time;

    /**
     * @brief The absolute maximum delta time allowed (prevents the "Spiral of Death").
     */
    float max_delta_time;

public:
    /**
     * @brief Constructs the calculator and starts the internal stopwatch.
     * @param max_dt The maximum allowed delta time in seconds. Defaults to 0.25s (roughly 4 FPS).
     */
    DeltaTimeCalculator(float max_dt = 0.25f) : max_delta_time(max_dt) {
        last_time = Clock::now();
    }

    /**
     * @brief Calculates the time passed since the last call to this function.
     * * MUST be called exactly once at the very start of your main game loop.
     * @return float The elapsed time (dt) in seconds.
     */
    float update() {
        TimePoint current_time = Clock::now();
        std::chrono::duration<float> elapsed = current_time - last_time;

        // Save the current time for the next frame's calculation
        last_time = current_time;

        float dt = elapsed.count();

        // Safety Check: Cap delta time.
        // If the game freezes for 2 seconds, we cap it to max_dt so the
        // physics engine doesn't try to simulate 2 seconds instantly and explode.
        if (dt > max_delta_time) {
            dt = max_delta_time;
        }

        return dt;
    }

    /**
     * @brief Manually resets the stopwatch.
     * * Useful when unpausing the game to prevent a massive dt spike.
     */
    void reset() {
        last_time = Clock::now();
    }
};