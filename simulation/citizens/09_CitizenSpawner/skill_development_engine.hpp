#pragma once

#include <entt/entt.hpp>
#include <algorithm>
#include <cmath>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/fixed_point.hpp"
#include "../../../core/04_Types/economic_constants.hpp"

#include "../08_Citizencomponent/employment_component.hpp"
#include "../08_Citizencomponent/education_component.hpp"
#include "../09_CitizenSpawner/daily_routine_executor.hpp" // For RoutineStateComponent

/**
 * @file skill_development_engine.hpp
 * @brief Subsystem 110: Accumulates citizen skill points and handles promotions.
 */

struct PromotionEligibleEvent {
    EntityID citizen;
    Population::JobType job_type;
    uint32_t new_level;
};

class SkillDevelopmentEngine {
public:
    /**
     * @brief Steps the skill development for all citizens currently at work.
     */
    void update(ECSWorld& world, EventPublisher& publisher) {
        auto view = world.registry.view<
            Population::EmploymentComponent,
            Population::EducationComponent,
            Population::RoutineStateComponent
        >();

        // Maximum achievable skill in the engine
        constexpr float MAX_SKILL = 10.0f;
        // Base skill gain per tick at work
        constexpr float BASE_GAIN = 0.0002f;

        for (auto raw_id : view) {
            auto& emp = view.get<Population::EmploymentComponent>(raw_id);
            auto& edu = view.get<Population::EducationComponent>(raw_id);
            auto& routine = view.get<Population::RoutineStateComponent>(raw_id);
            EntityID citizen{static_cast<uint32_t>(raw_id)};

            // Citizens only learn while actively working on the clock
            if (routine.current_state != Population::ActivityState::Working || emp.status != Population::EmploymentStatus::Employed) {
                continue;
            }

            // --- 1. Determine Active Job Category ---
            // In a full implementation, EmploymentComponent would explicitly store the JobCategory.
            // For now, we map JobType to a plausible Category to accumulate the right skill.
            Population::JobCategory active_category = Population::JobCategory::Manufacturing;
            if (emp.job_type == Population::JobType::Service) active_category = Population::JobCategory::Retail;
            else if (emp.job_type == Population::JobType::Professional) active_category = Population::JobCategory::Administration;
            else if (emp.job_type == Population::JobType::Management) active_category = Population::JobCategory::Research;

            // --- 2. Calculate Current Level ---
            // We divide the 10.0 scale into 5 discrete promotion tiers (0, 2, 4, 6, 8)
            float current_skill = edu.skill_levels[static_cast<size_t>(active_category)];
            uint32_t old_level = static_cast<uint32_t>(current_skill / 2.0f);

            // --- 3. Diminishing Returns Accumulation ---
            float quality_of_workplace = 1.0f; // Could be queried from the Employer entity later
            float skill_gain = BASE_GAIN * quality_of_workplace * (1.0f - (current_skill / MAX_SKILL));

            // Add experience using the existing component helper
            edu.add_job_experience(active_category, skill_gain);
            float new_skill = edu.skill_levels[static_cast<size_t>(active_category)];

            // --- 4. Promotion Threshold Check ---
            uint32_t new_level = static_cast<uint32_t>(new_skill / 2.0f);

            // If crossing a 2.0f boundary (e.g., 1.99 -> 2.00)
            if (new_level > old_level && new_level <= 5) {

                // Fire the macroeconomic event
                publisher.publish(PromotionEligibleEvent{citizen, emp.job_type, new_level});

                // Apply the immediate wage premium (8% bump per level)
                // Using a hardcoded 0.08f fallback just in case Labour::SKILL_PREMIUM_COEFFICIENT isn't strictly defined
                float premium = 0.08f;
                float current_wage = emp.wage.toFloat();
                current_wage *= (1.0f + premium);
                emp.wage = Fixed32(current_wage);
            }
        }
    }
};