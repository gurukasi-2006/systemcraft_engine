#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include "../01_ECS_core/ecs_world.hpp"

/**
 * @file system_executor.hpp
 * @brief Manages the registration and ordered execution of all game systems.
 */

/**
 * @class ISystem
 * @brief The base interface contract that all game systems MUST inherit from.
 */
class ISystem {
public:
    virtual ~ISystem() = default;

    /**
     * @brief The core logic loop for the system.
     * @param world The master ECS World containing all game data.
     * @param dt The delta time (or fixed step) since the last frame.
     */
    virtual void update(ECSWorld& world, float dt) = 0;
};

/**
 * @class SystemExecutor
 * @brief Maintains an ordered list of systems and triggers them sequentially.
 */
class SystemExecutor {
private:
    /**
     * @var systems
     * @brief A dynamic array holding unique pointers to every registered system.
     */
    std::vector<std::unique_ptr<ISystem>> systems;

public:
    /**
     * @brief Registers a new game system.
     * * The exact order you call this is the exact order they will execute!
     * @tparam T The specific system class (e.g., PhysicsSystem).
     * @tparam Args Any arguments needed to construct the system.
     */
    template<typename T, typename... Args>
    void addSystem(Args&&... args) {
        systems.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    /**
     * @brief Triggers the update() function on every registered system in order.
     * @param world The master ECS World.
     * @param dt The time elapsed since the last execution.
     */
    void updateAll(ECSWorld& world, float dt) {
        for (auto& system : systems) {
            system->update(world, dt);
        }
    }
};