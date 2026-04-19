#pragma once

#include <entt/entt.hpp>
#include <cmath>
#include <algorithm>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/04_Types/tile_coord.hpp"

#include "../10_NeedsSystem/need_definitions.hpp" // Subsystem 115
#include "../09_CitizenSpawner/citizen_spawner.hpp" // For PositionComponent

/**
 * @file healthcare_access_evaluator.hpp
 * @brief Subsystem 118: Evaluates hospital accessibility and penalizes overcrowded facilities.
 */

namespace Economy {
    /**
     * @struct HealthcareFacilityComponent
     * @brief Represents a Clinic, Hospital, or Research Center.
     */
    struct HealthcareFacilityComponent {
        float supply_radius{20.0f};  // Coverage in tiles
        float capacity{100.0f};      // Maximum optimal patient load
        float queue_length{0.0f};    // Current number of patients seeking care
        float facility_quality{1.0f};// Rated quality from 0.0 (awful) to 1.0 (state-of-the-art)
    };
}

class HealthcareAccessEvaluator {
public:
    /**
     * @brief Checks hospital proximity and applies health satisfaction replenishment based on queue times.
     * @param world The ECS master world.
     */
    void update(ECSWorld& world) {
        // Base mathematical constant from design spec
        constexpr float BASE_REPLENISH_RATE = 0.003f;

        auto facility_view = world.registry.view<PositionComponent, Economy::HealthcareFacilityComponent>();
        auto citizen_view = world.registry.view<PositionComponent, Population::NeedsComponent>();

        for (auto cit_id : citizen_view) {
            const auto& cit_pos = citizen_view.get<PositionComponent>(cit_id);
            auto& needs = citizen_view.get<Population::NeedsComponent>(cit_id);

            float current_health = needs.get_need(Population::NeedType::Healthcare);

            // Skip fully satisfied citizens to save CPU cycles
            if (current_health >= 100.0f) {
                continue;
            }

            float best_replenish = 0.0f;

            // --- Spatial & Queue Query ---
            for (auto fac_id : facility_view) {
                const auto& fac_pos = facility_view.get<PositionComponent>(fac_id);
                const auto& fac_data = facility_view.get<Economy::HealthcareFacilityComponent>(fac_id);

                // Euclidean distance check
                float dx = static_cast<float>(cit_pos.coord.x - fac_pos.coord.x);
                float dy = static_cast<float>(cit_pos.coord.y - fac_pos.coord.y);
                float distance = std::sqrt((dx * dx) + (dy * dy));

                if (distance <= fac_data.supply_radius) {
                    // Prevent divide-by-zero on uninitialized facilities
                    float safe_capacity = std::max(1.0f, fac_data.capacity);

                    // Queue Factor: Drops as queue approaches capacity. Clamps at a minimum 0.1 (10%) efficiency.
                    float queue_ratio = fac_data.queue_length / safe_capacity;
                    float queue_factor = std::max(0.1f, 1.0f - queue_ratio);

                    // Effective Quality translates facility tech level and overcrowding into a final multiplier
                    float effective_qual = fac_data.facility_quality * queue_factor;

                    // Final replenish calculated per tick
                    float replenish = effective_qual * BASE_REPLENISH_RATE;

                    // Citizens will logically choose the hospital that can treat them the fastest/best
                    if (replenish > best_replenish) {
                        best_replenish = replenish;
                    }
                }
            }

            // Apply the actual healthcare treatment to the citizen's needs
            if (best_replenish > 0.0f) {
                needs.add_to_need(Population::NeedType::Healthcare, best_replenish);
            }
        }
    }
};