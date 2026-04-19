#pragma once

#include <random>
#include <algorithm>
#include <entt/entt.hpp>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp" // <-- Swapped to Publisher!
#include "../../../core/04_Types/time_constants.hpp"

#include "../08_Citizencomponent/age_lifecycle_component.hpp"
#include "../08_Citizencomponent/health_component.hpp"
#include "../09_CitizenSpawner/age_progression_system.hpp"
/**
 * @file death_handler.hpp
 * @brief Subsystem 104: Evaluates probabilistic mortality per tick.
 * @details Uses deferred publishing to queue cleanup logic (housing, jobs) for the next frame,
 * preventing lag spikes during mass-casualty events (plagues, famines).
 */

class DeathHandler {
public:
    static float calculate_mortality_probability(float age_years, float physical_health, float hazard_factor = 0.0f) {
        float age_factor = std::max(0.0f, (age_years - 60.0f) / 40.0f);
        float health_penalty = (100.0f - physical_health) / 100.0f;
        return 0.000012f * (1.0f + (age_factor * 4.0f) + (health_penalty * 2.0f) + hazard_factor);
    }

    /**
     * @brief Rolls mortality probability for all citizens and pushes death events to the Publisher.
     * @param world The ECS world containing citizens.
     * @param publisher The global megaphone to queue events for next tick.
     * @param rng Deterministic random number generator.
     */
    void update(ECSWorld& world, EventPublisher& publisher, std::mt19937& rng) {
        auto view = world.registry.view<Population::AgeLifecycleComponent, Population::HealthComponent>();
        std::uniform_real_distribution<float> roll_dist(0.0f, 1.0f);

        for (auto raw_id : view) {
            auto& age_comp = view.get<Population::AgeLifecycleComponent>(raw_id);
            auto& health_comp = view.get<Population::HealthComponent>(raw_id);
            EntityID citizen{static_cast<uint32_t>(raw_id)};

            float age_years = static_cast<float>(age_comp.current_age_ticks) / TimeConstants::TICKS_PER_YEAR;
            float hazard_factor = 0.0f;

            float p_death = calculate_mortality_probability(age_years, health_comp.physical_health, hazard_factor);

            if (roll_dist(rng) < p_death) {
                // Publish to the queue. The DeferredDispatcher will execute this at the end of the tick.
                publisher.publish(CitizenDiedEvent{citizen});
            }
        }
    }
};