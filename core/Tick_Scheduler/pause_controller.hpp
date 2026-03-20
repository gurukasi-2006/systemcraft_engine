#pragma once

/**
 * @file pause_controller.hpp
 * @brief Manages the simulation's active state, allowing time to freeze.
 */

class PauseController {
private:
    /**
     * @brief The core state flag. If true, the simulation logic halts.
     */
    bool is_paused;

public:
    /**
     * @brief Constructs the controller. Games usually start unpaused.
     * @param start_paused Optional flag to boot the game in a frozen state.
     */
    PauseController(bool start_paused = false) : is_paused(start_paused) {}

    /**
     * @brief Freezes the simulation.
     */
    void pause() {
        is_paused = true;
    }

    /**
     * @brief Resumes the simulation.
     */
    void resume() {
        is_paused = false;
    }

    /**
     * @brief Flips the current state (perfect for a UI Pause button or hitting Escape).
     */
    void toggle() {
        is_paused = !is_paused;
    }

    /**
     * @brief Checks if the game is currently frozen.
     * @return true if paused, false if running.
     */
    bool isPaused() const {
        return is_paused;
    }
};