#pragma once

#include <algorithm>

/**
 * @file speed_multiplier.hpp
 * @brief Controls the flow of time in the simulation (e.g., 1x, 5x, 20x).
 */

class SpeedMultiplier {
private:
    /**
     * @brief The current speed scale. 1.0 is real-time, 5.0 is fast-forward.
     */
    float current_multiplier;

public:
    /**
     * @brief Constructs the time scaler. Defaults to 1x normal speed.
     */
    SpeedMultiplier(float start_speed = 1.0f) : current_multiplier(std::max(0.0f, start_speed)) {}

    /**
     * @brief Sets the new simulation speed.
     * @param new_speed The target multiplier. Negative values are clamped to 0 (Paused).
     */
    void setSpeed(float new_speed) {
        current_multiplier = std::max(0.0f, new_speed);
    }

    /**
     * @brief Retrieves the current speed multiplier.
     */
    float getSpeed() const {
        return current_multiplier;
    }

    /**
     * @brief Scales a real-world delta time by the multiplier.
     * * This is the magic function that accelerates your Fixed-Rate Scheduler and Physics loop!
     * @param real_world_dt The actual time passed in seconds.
     * @return The scaled simulation time.
     */
    float scale(float real_world_dt) const {
        return real_world_dt * current_multiplier;
    }
};