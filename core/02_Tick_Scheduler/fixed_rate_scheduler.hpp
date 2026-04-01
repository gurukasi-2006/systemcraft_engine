#pragma once

#include <vector>
#include <memory>

#include "system_executor.hpp"
#include "tick_counter.hpp"
#include "../04_Types/engine_assert.hpp"

/**
 * @file fixed_rate_scheduler.hpp
 * @brief Manages game systems that only need to execute intermittently.
 */

class FixedRateScheduler {
private:
    /**
     * @brief Internal wrapper linking a system to its specific timing rules.
     */
    struct ScheduledSystem {
        uint64_t tick_interval;
        uint64_t phase_offset;
        std::unique_ptr<ISystem> system;
    };

    std::vector<ScheduledSystem> systems;

public:
    /**
     * @brief Registers a system to run only every N ticks.
     * @tparam T The specific system class (e.g., DemographicsSystem).
     * @param interval How many ticks between executions (e.g., 30 = once every 30 ticks).
     * @param offset (Optional) Delays the execution by N ticks to prevent CPU spikes.
     */
    template<typename T, typename... Args>
    void addSystem(uint64_t interval, uint64_t offset = 0, Args&&... args) {

        // Guard against integer modulo-by-zero crashes!
        ENGINE_ASSERT(interval > 0, "FixedRateScheduler: tick_interval cannot be zero.");

        systems.push_back({
            interval,
            offset,
            std::make_unique<T>(std::forward<Args>(args)...)
        });
    }

    /**
     * @brief Evaluates all registered systems and runs them IF it is their scheduled tick.
     * @param world The master ECS World.
     * @param dt The delta time.
     * @param clock The master simulation tick counter.
     */
    void updateAll(ECSWorld& world, float dt, const TickCounter& clock) {
        uint64_t current_tick = clock.get();

        for (auto& sched : systems) {
            // Evaluates execution phase based on the current tick and registered interval
            if (current_tick >= sched.phase_offset &&
               (current_tick - sched.phase_offset) % sched.tick_interval == 0) {
                sched.system->update(world, dt);
            }
        }
    }
};
