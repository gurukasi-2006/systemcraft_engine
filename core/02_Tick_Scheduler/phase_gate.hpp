#pragma once

#include <map>
#include <vector>
#include <memory>
#include "system_executor.hpp"

/**
 * @file phase_gate.hpp
 * @brief Groups systems into strict execution phases to prevent race conditions.
 */

class PhaseGate {
private:
    /**
     * @brief A map of Phase IDs to their respective bucket of systems.
     * * std::map automatically sorts from lowest key to highest key.
     */
    std::map<int, std::vector<std::unique_ptr<ISystem>>> phases;

public:
    /**
     * @brief Registers a system into a specific execution bucket.
     * @tparam T The specific system class.
     * @param phase_id The bucket ID (e.g., 0 = Pre-Update, 1 = Logic, 2 = Render).
     */
    template<typename T, typename... Args>
    void addSystemToPhase(int phase_id, Args&&... args) {
        phases[phase_id].push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    /**
     * @brief Executes all phases in strict sequential order.
     * @param world The master ECS World.
     * @param dt The delta time.
     */
    void updateAll(ECSWorld& world, float dt) {
        for (auto& [phase_id, system_bucket] : phases) {

            for (auto& system : system_bucket) {
                system->update(world, dt);
            }
        }
    }
};