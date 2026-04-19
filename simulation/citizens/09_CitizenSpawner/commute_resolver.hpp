#pragma once

#include <algorithm>
#include <cmath>
#include <entt/entt.hpp>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../08_Citizencomponent/employment_component.hpp"

/**
 * @file commute_resolver.hpp
 * @brief Subsystem 107: Calculates shortest transport routes, costs, and satisfaction for employed citizens.
 */

namespace Population {
    /**
     * @enum TransportMode
     * @brief The modes of transit available in the city.
     */
    enum class TransportMode : uint8_t {
        Walking = 0,
        Road,
        Highway,
        Rail
    };

    /**
     * @struct CommuteComponent
     * @brief Stores the cached route data for a citizen's daily journey to work.
     */
    struct CommuteComponent {
        TransportMode mode{TransportMode::Walking};
        float commute_ticks{0.0f};
        float daily_cost{0.0f};
        float transport_satisfaction{100.0f};

        // Dirty flag pattern: Only re-run heavy pathfinding if home or job changes!
        bool requires_recalculation{true};
    };
}

class CommuteResolver {
public:
    // Lookup table for transport speeds (tiles per tick)
    static constexpr float get_speed(Population::TransportMode mode) {
        switch(mode) {
            case Population::TransportMode::Road: return 2.5f;
            case Population::TransportMode::Highway: return 4.0f;
            case Population::TransportMode::Rail: return 5.0f;
            case Population::TransportMode::Walking:
            default: return 0.8f;
        }
    }

    // Lookup table for fuel/ticket costs per tick
    static constexpr float get_cost(Population::TransportMode mode) {
        switch(mode) {
            case Population::TransportMode::Road: return 0.5f;    // Personal car fuel
            case Population::TransportMode::Highway: return 0.8f; // Higher fuel burn
            case Population::TransportMode::Rail: return 0.2f;    // Mass transit economy of scale
            case Population::TransportMode::Walking:
            default: return 0.0f;                                 // Walking is free!
        }
    }

    /**
     * @brief Evaluates and caches the optimal commute path for all employed citizens flagged for recalculation.
     */
    void update(ECSWorld& world) {
        auto view = world.registry.view<Population::EmploymentComponent, Population::CommuteComponent>();

        for (auto raw_id : view) {
            auto& emp = view.get<Population::EmploymentComponent>(raw_id);
            auto& commute = view.get<Population::CommuteComponent>(raw_id);

            // Only actively employed citizens need a route to work
            if (emp.status != Population::EmploymentStatus::Employed) {
                continue;
            }

            // Skip heavy calculations if the route is already cached and unchanged
            if (!commute.requires_recalculation) {
                continue;
            }

            // --- 1. Pathfinding Proxy ---
            // In a full engine, this calls the NavMesh/A* system.
            // We use the Euclidean distance cached during Subsystem 106 as the path length.
            float path_length = emp.commute_distance;

            // --- 2. Smart Mode Selection (Basic AI) ---
            // Citizens choose the fastest available transport infrastructure for their distance.
            if (path_length > 50.0f) {
                commute.mode = Population::TransportMode::Rail;
            } else if (path_length > 20.0f) {
                commute.mode = Population::TransportMode::Highway;
            } else if (path_length > 5.0f) {
                commute.mode = Population::TransportMode::Road;
            } else {
                commute.mode = Population::TransportMode::Walking;
            }

            float speed = get_speed(commute.mode);
            float cost_per_tick = get_cost(commute.mode);

            // --- 3. Mathematical Evaluation ---
            commute.commute_ticks = path_length / speed;
            commute.daily_cost = commute.commute_ticks * cost_per_tick;

            // Satisfaction Equation: 100 at zero commute, hitting 0 at ~33.3 ticks.
            commute.transport_satisfaction = std::max(0.0f, 100.0f - (commute.commute_ticks * 3.0f));

            // Cache the result
            commute.requires_recalculation = false;
        }
    }
};