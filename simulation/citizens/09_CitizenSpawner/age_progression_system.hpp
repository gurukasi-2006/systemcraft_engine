#pragma once

#include <entt/entt.hpp>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/04_Types/time_constants.hpp"
#include "../../../core/03_Event_Bus/immediate_dispatcher.hpp"
#include "../08_Citizencomponent/age_lifecycle_component.hpp"

/**
 * @file age_progression_engine.hpp
 * @brief Subsystem 103: Increments citizen age each tick and broadcasts lifecycle milestones.
 */

// --- Lifecycle Events ---
// Broadcasted to the Event Bus so other systems (Education, Employment, Housing) can react.
struct AdultHoodReachedEvent {
    EntityID citizen;
};

struct RetirementAgeEvent {
    EntityID citizen;
};

struct CitizenDiedEvent {
    EntityID citizen;
};
// ------------------------

class AgeProgressionEngine {
public:
    /**
     * @brief Steps the biological age of all citizens forward by 1 tick.
     * @param world The master ECS world containing the citizens.
     * @param dispatcher The Event Bus dispatcher to fire milestone events.
     */
    void update(ECSWorld& world, ImmediateDispatcher& dispatcher) {
        // Query every entity that has an Age component
        auto view = world.registry.view<Population::AgeLifecycleComponent>();

        for (auto raw_id : view) {
            auto& age_comp = view.get<Population::AgeLifecycleComponent>(raw_id);
            EntityID citizen{static_cast<uint32_t>(raw_id)};

            // 1. Cache the previous stage to detect state transitions
            Population::LifeStage prev_stage = age_comp.stage;

            // 2. Increment biological age
            age_comp.current_age_ticks += 1;
            uint64_t age_years = age_comp.current_age_ticks / TimeConstants::TICKS_PER_YEAR;

            // 3. Threshold Event Checks
            if (age_years >= 18 && prev_stage == Population::LifeStage::Child) {
                // Transition to Workforce
                age_comp.stage = Population::LifeStage::Adult;
                dispatcher.dispatch(AdultHoodReachedEvent{citizen});
            }
            else if (age_years >= 65 && prev_stage == Population::LifeStage::Adult) {
                // Transition to Retirement
                age_comp.stage = Population::LifeStage::Elder;
                dispatcher.dispatch(RetirementAgeEvent{citizen});
            }

            // 4. Mortality Check
            if (age_years >= age_comp.expected_lifespan_years) {
                // Fire the death event. The actual Entity destruction is usually handled
                // by a dedicated Cleanup/Graveyard system listening to this event!
                dispatcher.dispatch(CitizenDiedEvent{citizen});
            }
        }
    }
};