#pragma once

#include <entt/entt.hpp>
#include <cmath>
#include <algorithm>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/04_Types/tile_coord.hpp"

#include "../10_NeedsSystem/need_definitions.hpp" // Subsystem 115
#include "../09_CitizenSpawner/citizen_spawner.hpp" // For PositionComponent

/**
 * @file food_access_evaluator.hpp
 * @brief Subsystem 117: Checks market proximity and stock to replenish citizen food satisfaction.
 */

namespace Economy {
    /**
     * @struct FoodDistributionComponent
     * @brief Represents a Market, Grocery Store, or Ration Center.
     * @details Attached to building entities to project a food supply radius.
     */
    struct FoodDistributionComponent {
        float supply_radius{10.0f}; // Coverage in tiles
        float current_stock{0.0f};  // Raw caloric/food units available
        float daily_demand{1.0f};   // Estimated local demand (prevents divide-by-zero)
    };
}

class FoodAccessEvaluator {
public:
    /**
     * @brief Evaluates citizen proximity to markets and replenishes food satisfaction.
     * @param world The ECS master world.
     */
    void update(ECSWorld& world) {
        // Base mathematical constants from design spec
        constexpr float BASE_DECAY = 0.014f;
        constexpr float MAX_REPLENISH = BASE_DECAY * 1.8f; // 0.0252f per tick

        // Fetch all active markets in the city
        auto market_view = world.registry.view<PositionComponent, Economy::FoodDistributionComponent>();

        // Fetch all citizens with needs and physical locations
        auto citizen_view = world.registry.view<PositionComponent, Population::NeedsComponent>();

        for (auto cit_id : citizen_view) {
            const auto& cit_pos = citizen_view.get<PositionComponent>(cit_id);
            auto& needs = citizen_view.get<Population::NeedsComponent>(cit_id);

            float current_food = needs.get_need(Population::NeedType::Food);

            // If they are completely full, skip expensive spatial checks
            if (current_food >= 100.0f) {
                continue;
            }

            float best_replenish = 0.0f;

            // --- Spatial Query ---
            // Note: In a heavily optimized pass, this would use Subsystem 05's SpatialIndex.
            // We iterate available markets to find the one providing the best access quality.
            for (auto market_id : market_view) {
                const auto& mkt_pos = market_view.get<PositionComponent>(market_id);
                const auto& mkt_data = market_view.get<Economy::FoodDistributionComponent>(market_id);

                // Euclidean distance check
                float dx = static_cast<float>(cit_pos.coord.x - mkt_pos.coord.x);
                float dy = static_cast<float>(cit_pos.coord.y - mkt_pos.coord.y);
                float distance = std::sqrt((dx * dx) + (dy * dy));

                if (distance <= mkt_data.supply_radius && mkt_data.current_stock > 0.0f) {

                    // Access Quality: 1.0 if stock exceeds demand, fractions if in shortage
                    float access_quality = std::min(1.0f, mkt_data.current_stock / std::max(1.0f, mkt_data.daily_demand));

                    // Calculate potential replenish amount
                    float replenish = std::min(MAX_REPLENISH, 100.0f - current_food) * access_quality;

                    // Citizens will shop at the market that offers the most food
                    if (replenish > best_replenish) {
                        best_replenish = replenish;
                    }
                }
            }

            // Apply the actual caloric intake to the citizen's needs
            if (best_replenish > 0.0f) {
                needs.add_to_need(Population::NeedType::Food, best_replenish);
            }

            // If best_replenish == 0.0f, they effectively starve this tick
            // (decay will be handled separately by Subsystem 116).
        }
    }
};