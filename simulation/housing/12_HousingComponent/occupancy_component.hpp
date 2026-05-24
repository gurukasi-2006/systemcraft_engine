#pragma once

#include <vector>
#include <algorithm>
#include <cstdint>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/entity_id.hpp"

#include "../../citizens/08_Citizencomponent/health_component.hpp"

/**
 * @file occupancy_component.hpp
 * @brief Subsystem 143: Tracks building residents and applies emergent health penalties for overcrowding.
 */

namespace Housing {

    /**
     * @struct OverOccupiedEvent
     * @brief Fired when a building exceeds its safe maximum capacity, alerting urban planners.
     */
    struct OverOccupiedEvent {
        EntityID building_id;
        float overcrowding_ratio;
        uint32_t current_occupants;
        uint32_t max_capacity;
    };

    /**
     * @struct OccupancyComponent
     * @brief Manages the tenant list and calculates emergent overcrowding pressures.
     */
    struct OccupancyComponent {
        std::vector<EntityID> occupant_list;
        uint32_t max_capacity{4};

        /**
         * @brief Returns the strict occupancy rate [0.0, ...].
         */
        float get_occupancy_rate() const {
            if (max_capacity == 0) return 0.0f;
            return static_cast<float>(occupant_list.size()) / static_cast<float>(max_capacity);
        }

        /**
         * @brief Returns the overcrowding ratio, clamped to a minimum of 1.0 for math safety.
         */
        float get_overcrowding_ratio() const {
            return std::max(1.0f, get_occupancy_rate());
        }

        /**
         * @brief Evaluates the current capacity and applies health penalties to occupants if overcrowded.
         * @param world The ECS World to locate occupant entities.
         * @param publisher The Event Bus to fire overcrowding alerts.
         * @param self_id The EntityID of this building.
         */
        void process_occupancy(ECSWorld& world, EventPublisher& publisher, EntityID self_id) {
            float overcrowding_ratio = get_overcrowding_ratio();

            // Self-Limiting Mechanic: Overcrowding triggers disease and mortality
            if (overcrowding_ratio > 1.0f) {

                // 1. Fire Macroeconomic Alert
                publisher.publish(OverOccupiedEvent{
                    self_id,
                    overcrowding_ratio,
                    static_cast<uint32_t>(occupant_list.size()),
                    max_capacity
                });

                // 2. Apply Microeconomic Health Penalties
                // Formula: (overcrowding_ratio - 1.0) * 0.005
                float penalty = (overcrowding_ratio - 1.0f) * 0.005f;

                for (EntityID cit_id : occupant_list) {
                    entt::entity raw = static_cast<entt::entity>(cit_id.raw_id);

                    if (world.registry.valid(raw) && world.registry.all_of<Population::HealthComponent>(raw)) {
                        auto& health = world.registry.get<Population::HealthComponent>(raw);

                        // C++20 Concept fallback for however you named the health float in Phase 8
                        if constexpr (requires { health.physical_health; }) {
                            health.physical_health -= penalty;
                            if (health.physical_health < 0.0f) health.physical_health = 0.0f;
                        } else if constexpr (requires { health.physical_health; }) {
                            health.physical_health -= penalty;
                            if (health.physical_health < 0.0f) health.physical_health = 0.0f;
                        }
                    }
                }
            }
        }
    };
}