#pragma once

#include <vector>
#include <memory>
#include "system_executor.hpp"

/**
 * @file dirty_flag_scheduler.hpp
 * @brief Skips system execution unless specifically flagged as "dirty" (modified).
 */

class DirtyFlagScheduler {
private:
    /**
     * @brief Pairs a system with its specific memory state flag.
     */
    struct TrackedSystem {
        bool* dirty_flag;
        std::unique_ptr<ISystem> system;
    };

    std::vector<TrackedSystem> systems;

public:
    /**
     * @brief Registers a system and links it to a specific boolean flag.
     * @tparam T The specific system class (e.g., UIRenderSystem).
     * @param flag A pointer to the boolean tracking if the relevant data has changed.
     */
    template<typename T, typename... Args>
    void addSystem(bool* flag, Args&&... args) {
        systems.push_back({flag, std::make_unique<T>(std::forward<Args>(args)...)});
    }

    /**
     * @brief Evaluates the flags and only runs systems that need updating.
     * @param world The master ECS World.
     * @param dt The delta time.
     */
    void updateAll(ECSWorld& world, float dt) {
        for (auto& tracked : systems) {

            if (tracked.dirty_flag != nullptr && *(tracked.dirty_flag) == true) {


                tracked.system->update(world, dt);


                *(tracked.dirty_flag) = false;
            }
        }
    }
};