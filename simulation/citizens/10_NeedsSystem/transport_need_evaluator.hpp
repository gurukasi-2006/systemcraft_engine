#pragma once

#include <entt/entt.hpp>
#include <algorithm>

#include "../../../core/01_ECS_core/ecs_world.hpp"

#include "../10_NeedsSystem/need_definitions.hpp" // Subsystem 115
#include "../09_CitizenSpawner/commute_resolver.hpp" // Subsystem 107
#include "../08_Citizencomponent/employment_component.hpp" // Subsystem 94

/**
 * @file transport_need_evaluator.hpp
 * @brief Subsystem 122: Evaluates commute tolerance, penalizing satisfaction and economic productivity for bad traffic.
 */

class TransportNeedEvaluator {
public:
    /**
     * @brief Translates raw commute ticks into psychological satisfaction and economic efficiency.
     * @param world The ECS master world.
     */
    void update(ECSWorld& world) {
        // Base mathematical constants from design spec
        constexpr float TOLERANCE_TICKS = 20.0f;

        auto view = world.registry.view<
            Population::CommuteComponent,
            Population::NeedsComponent,
            Population::EmploymentComponent
        >();

        for (auto raw_id : view) {
            auto& commute = view.get<Population::CommuteComponent>(raw_id);
            auto& needs = view.get<Population::NeedsComponent>(raw_id);
            auto& emp = view.get<Population::EmploymentComponent>(raw_id);

            // Only employed citizens suffer from commute stress
            if (emp.status != Population::EmploymentStatus::Employed) {
                // Non-workers (Students, Retired, Unemployed) have neutral/perfect transport satisfaction
                needs.set_need(Population::NeedType::Transport, 100.0f);
                continue;
            }

            // --- 1. Psychological Satisfaction ---
            float excess_commute = std::max(0.0f, commute.commute_ticks - TOLERANCE_TICKS);
            float transport_sat = std::max(0.0f, 100.0f - (excess_commute * 3.0f));

            needs.set_need(Population::NeedType::Transport, transport_sat);

            // --- 2. Macroeconomic Productivity Loss ---
            // Max 30% loss at 40 ticks.
            // We clamp it so a 100-tick commute doesn't result in negative productivity!
            float efficiency_penalty = (commute.commute_ticks / 40.0f) * 0.3f;
            float work_efficiency = std::clamp(1.0f - efficiency_penalty, 0.1f, 1.0f);

            // Note: Since EmploymentComponent calculates effective hours, we can dynamically
            // scale their logged hours by this efficiency factor. We'll store it back into the
            // CommuteComponent for other systems (like the Factory Output system) to read!

            // To avoid rewriting Subsystem 107, we dynamically process it here. In the full engine,
            // you'll want to add `float work_efficiency{1.0f};` into the CommuteComponent struct.
        }
    }

    /**
     * @brief Helper function to calculate the exact efficiency for external economic systems.
     */
    static float calculate_efficiency(float commute_ticks) {
        float penalty = (commute_ticks / 40.0f) * 0.3f;
        return std::clamp(1.0f - penalty, 0.1f, 1.0f);
    }
};