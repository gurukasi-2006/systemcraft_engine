#pragma once

#include <entt/entt.hpp>
#include <algorithm>
#include <functional>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/04_Types/tile_coord.hpp"

#include "../10_NeedsSystem/need_definitions.hpp" // Subsystem 115
#include "../08_Citizencomponent/political_belief_component.hpp"
#include "../09_CitizenSpawner/citizen_spawner.hpp" // For PositionComponent

/**
 * @file safety_need_evaluator.hpp
 * @brief Subsystem 120: Evaluates safety needs and drives authoritarian political drift.
 */

namespace Environment {
    /**
     * @struct TileSecurityData
     * @brief Proxy data structure holding local grid threats (0.0 to 1.0).
     */
    struct TileSecurityData {
        float crime_rate{0.0f};
        float military_threat{0.0f};
    };
}

class SafetyNeedEvaluator {
public:
    // Uses a functional lookup so it can seamlessly connect to the Terrain/Crime system grid later
    using SecurityLookup = std::function<Environment::TileSecurityData(TileCoord)>;

    /**
     * @brief Evaluates localized threat levels, modifying safety satisfaction and political ideology.
     * @param world The ECS master world.
     * @param get_security_data A lambda or bound function returning the local threat level for a TileCoord.
     */
    void update(ECSWorld& world, const SecurityLookup& get_security_data) {
        auto view = world.registry.view<
            PositionComponent,
            Population::NeedsComponent,
            Population::PoliticalBeliefComponent
        >();

        for (auto raw_id : view) {
            const auto& pos = view.get<PositionComponent>(raw_id);
            auto& needs = view.get<Population::NeedsComponent>(raw_id);
            auto& pol = view.get<Population::PoliticalBeliefComponent>(raw_id);

            // Fetch local tile threat from the spatial grid
            Environment::TileSecurityData security = get_security_data(pos.coord);

            // Calculate composite threat level (Crime weighted slightly higher for daily life)
            float threat_level = std::clamp((security.crime_rate * 0.6f) + (security.military_threat * 0.4f), 0.0f, 1.0f);

            // --- 1. Needs Satisfaction Calculation ---
            // High threat reduces safety, low threat slowly recovers it
            float delta = ((1.0f - threat_level) * 0.003f) - (threat_level * 0.01f);

            // Apply delta (add_to_need inherently clamps 0 to 100)
            needs.add_to_need(Population::NeedType::Safety, delta);

            // --- 2. The Weimar Effect (Political Contagion) ---
            // Extreme fear of crime drives authoritarian political drift
            if (needs.get_need(Population::NeedType::Safety) < 30.0f) {
                // Social axis positive (+) = Authoritarianism
                pol.social_axis = std::clamp(pol.social_axis + 0.002f, -1.0f, 1.0f);
            }
        }
    }
};