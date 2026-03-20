#pragma once

#include <vector>
#include <memory>
#include "system_executor.hpp"

/**
 * @file late_update_bucket.hpp
 * @brief A dedicated execution queue guaranteed to run after all standard phases.
 */

class LateUpdateBucket {
private:
    /**
     * @brief A simple, ordered list of cleanup and aggregation systems.
     */
    std::vector<std::unique_ptr<ISystem>> systems;

public:
    /**
     * @brief Registers a system specifically for end-of-frame execution.
     * @tparam T The specific system class (e.g., DeathCleanupSystem).
     */
    template<typename T, typename... Args>
    void addSystem(Args&&... args) {
        systems.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    /**
     * @brief Triggers the update() function on all late-stage systems.
     * * MUST be called exactly once at the very end of your game loop tick.
     * @param world The master ECS World.
     * @param dt The time elapsed since the last execution.
     */
    void updateAll(ECSWorld& world, float dt) {
        for (auto& system : systems) {
            system->update(world, dt);
        }
    }
};