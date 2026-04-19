#pragma once

#include <entt/entt.hpp>
#include <cmath>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/04_Types/tile_coord.hpp"

#include "../10_NeedsSystem/need_definitions.hpp" // Subsystem 115
#include "../09_CitizenSpawner/citizen_spawner.hpp" // For PositionComponent
#include "../08_Citizencomponent/age_lifecycle_component.hpp"
#include "../08_Citizencomponent/education_component.hpp"

/**
 * @file education_access_evaluator.hpp
 * @brief Subsystem 119: Evaluates childhood access to schools. Inflicts permanent generational damage if deprived.
 */

namespace Economy {
    /**
     * @struct EducationalFacilityComponent
     * @brief Represents a Primary or Secondary School.
     */
    struct EducationalFacilityComponent {
        float supply_radius{30.0f};  // Commute distance for kids
        uint32_t capacity{500};      // Maximum available desks
        uint32_t enrolled_this_tick{0}; // Tracks live capacity usage
    };
}

namespace Population {
    /**
     * @struct LearningAptitudeComponent
     * @brief Supplemental component tracking permanent neurological/educational damage.
     */
    struct LearningAptitudeComponent {
        float skill_dev_multiplier{1.0f}; // Drops to 0.5 if deprived of childhood education!
    };
}

class EducationAccessEvaluator {
public:
    void update(ECSWorld& world) {
        auto school_view = world.registry.view<PositionComponent, Economy::EducationalFacilityComponent>();

        // 1. Reset school enrollment counters for the new tick
        for (auto fac_id : school_view) {
            school_view.get<Economy::EducationalFacilityComponent>(fac_id).enrolled_this_tick = 0;
        }

        auto citizen_view = world.registry.view<
            PositionComponent,
            Population::NeedsComponent,
            Population::AgeLifecycleComponent,
            Population::EducationComponent
        >();

        for (auto cit_id : citizen_view) {
            auto& age = citizen_view.get<Population::AgeLifecycleComponent>(cit_id);

            // Education gating: Only minors need active schooling access
            if (age.stage != Population::LifeStage::Child) {
                continue;
            }

            // Ensure they have the aptitude tracker attached
            if (!world.registry.all_of<Population::LearningAptitudeComponent>(cit_id)) {
                world.registry.emplace<Population::LearningAptitudeComponent>(cit_id);
            }

            const auto& cit_pos = citizen_view.get<PositionComponent>(cit_id);
            auto& needs = citizen_view.get<Population::NeedsComponent>(cit_id);
            auto& edu = citizen_view.get<Population::EducationComponent>(cit_id);
            auto& aptitude = world.registry.get<Population::LearningAptitudeComponent>(cit_id);

            bool found_seat = false;

            // 2. Search for an available desk in a reachable school
            for (auto fac_id : school_view) {
                const auto& sch_pos = school_view.get<PositionComponent>(fac_id);
                auto& sch_data = school_view.get<Economy::EducationalFacilityComponent>(fac_id);

                if (sch_data.enrolled_this_tick < sch_data.capacity) {
                    float dx = static_cast<float>(cit_pos.coord.x - sch_pos.coord.x);
                    float dy = static_cast<float>(cit_pos.coord.y - sch_pos.coord.y);
                    float distance = std::sqrt((dx * dx) + (dy * dy));

                    if (distance <= sch_data.supply_radius) {
                        sch_data.enrolled_this_tick++;
                        found_seat = true;
                        break;
                    }
                }
            }

            // 3. Apply the Generational Consequences
            if (found_seat) {
                needs.add_to_need(Population::NeedType::Education, 0.1f);
                edu.enrolled = true;
            } else {
                needs.add_to_need(Population::NeedType::Education, -0.0005f);
                edu.enrolled = false;

                // THE GENERATIONAL DAMAGE PENALTY
                // This citizen will now learn skills at half speed for the rest of their adult life.
                aptitude.skill_dev_multiplier = 0.5f;
            }
        }
    }
};