#pragma once

#include <entt/entt.hpp>
#include <cmath>
#include <algorithm>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/04_Types/tile_coord.hpp"

#include "../10_NeedsSystem/need_definitions.hpp" // Subsystem 115
#include "../09_CitizenSpawner/citizen_spawner.hpp" // For PositionComponent

/**
 * @file leisure_need_evaluator.hpp
 * @brief Subsystem 121: Evaluates access to cultural venues and updates long-term mental health.
 */

namespace Economy {
    /**
     * @struct LeisureFacilityComponent
     * @brief Represents a Park, Museum, Theater, or Recreation Center.
     */
    struct LeisureFacilityComponent {
        float supply_radius{30.0f}; // Maximum distance citizens will travel for this venue
        float quality{1.0f};        // 1.0 = Basic Park, 5.0 = World-Class Museum
    };
}

namespace Population {
    /**
     * @struct MentalHealthComponent
     * @brief Tracks the citizen's long-term psychological well-being.
     */
    struct MentalHealthComponent {
        float current_score{50.0f}; // 0.0 (Severe Depression) to 100.0 (Perfectly Content)
    };
}

class LeisureNeedEvaluator {
public:
    /**
     * @brief Evaluates proximity to leisure venues, replenishes satisfaction, and updates mental health.
     * @param world The ECS master world.
     */
    void update(ECSWorld& world) {
        auto venue_view = world.registry.view<PositionComponent, Economy::LeisureFacilityComponent>();

        auto citizen_view = world.registry.view<
            PositionComponent,
            Population::NeedsComponent
        >();

        for (auto cit_id : citizen_view) {
            const auto& cit_pos = citizen_view.get<PositionComponent>(cit_id);
            auto& needs = citizen_view.get<Population::NeedsComponent>(cit_id);

            // Ensure the citizen has a Mental Health tracker
            if (!world.registry.all_of<Population::MentalHealthComponent>(cit_id)) {
                world.registry.emplace<Population::MentalHealthComponent>(cit_id);
            }
            auto& mental_health = world.registry.get<Population::MentalHealthComponent>(cit_id);

            // --- 1. Calculate Local Venue Score ---
            float venue_score = 0.0f;

            for (auto fac_id : venue_view) {
                const auto& fac_pos = venue_view.get<PositionComponent>(fac_id);
                const auto& fac_data = venue_view.get<Economy::LeisureFacilityComponent>(fac_id);

                float dx = static_cast<float>(cit_pos.coord.x - fac_pos.coord.x);
                float dy = static_cast<float>(cit_pos.coord.y - fac_pos.coord.y);
                float distance = std::sqrt((dx * dx) + (dy * dy));

                // If within commute distance, add to the aggregate venue score
                if (distance <= fac_data.supply_radius) {
                    // Formula: quality * (1 - distance / max_radius)
                    venue_score += fac_data.quality * (1.0f - (distance / fac_data.supply_radius));
                }
            }

            // --- 2. Replenish Leisure Satisfaction ---
            if (venue_score > 0.0f) {
                // Max replenishment capped at 0.01 per tick
                float replenish = std::min(0.01f, venue_score * 0.005f);
                needs.add_to_need(Population::NeedType::Leisure, replenish);
            }

            // --- 3. The Mental Health Link ---
            // High leisure boosts mental health; low leisure decays it.
            float current_leisure = needs.get_need(Population::NeedType::Leisure);
            float mh_delta = (current_leisure - 50.0f) * 0.0002f;

            mental_health.current_score = std::clamp(mental_health.current_score + mh_delta, 0.0f, 100.0f);
        }
    }
};