#pragma once

#include <entt/entt.hpp>
#include <vector>
#include <algorithm>
#include <unordered_map>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/tile_coord.hpp"
#include "../../../core/04_Types/economic_constants.hpp"

#include "../10_NeedsSystem/need_definitions.hpp" // Subsystem 115
#include "../08_Citizencomponent/employment_component.hpp"
#include "../09_CitizenSpawner/citizen_spawner.hpp" // For PositionComponent

/**
 * @file poverty_classifier.hpp
 * @brief Subsystem 127: Evaluates multi-dimensional poverty and regional instability.
 */

namespace Welfare {

    // Global Constants from Economic Design
    //constexpr float POVERTY_INCOME_THRESHOLD = 0.50f;
    //constexpr float GINI_INSTABILITY_COEFFICIENT = 0.80f;

    /**
     * @struct PovertyComponent
     * @brief Tags a citizen as living in true multi-dimensional poverty.
     */
    struct PovertyComponent {
        bool is_poor{false};
    };

    /**
     * @struct RegionalInstabilityEvent
     * @brief Fired when a region's poverty rate exceeds the tolerance threshold.
     */
    struct RegionalInstabilityEvent {
        uint32_t region_id;
        float instability_delta;
        float poverty_rate;
    };

    class PovertyClassifier {
    private:
        uint32_t get_region_for_tile(TileCoord coord) const {
            return static_cast<uint32_t>(coord.x / 100);
        }

    public:
        void update(ECSWorld& world, EventPublisher& publisher) {
            auto view = world.registry.view<
                PositionComponent,
                Population::NeedsComponent,
                Population::EmploymentComponent
            >();

            // --- 1. Calculate National Median Income ---
            std::vector<float> all_incomes;
            all_incomes.reserve(view.size_hint());

            for (auto raw_id : view) {
                // Approximate monthly income based on wage
                float wage = view.get<Population::EmploymentComponent>(raw_id).wage.toFloat();
                all_incomes.push_back(wage);
            }

            float median_income = 0.0f;
            if (!all_incomes.empty()) {
                std::nth_element(all_incomes.begin(), all_incomes.begin() + all_incomes.size() / 2, all_incomes.end());
                median_income = all_incomes[all_incomes.size() / 2];
            }

            // --- 2. Evaluate Poverty & Aggregate Regionally ---
            struct RegionStats {
                uint32_t total_pop{0};
                uint32_t poor_pop{0};
            };
            std::unordered_map<uint32_t, RegionStats> regional_stats;

            for (auto raw_id : view) {
                const auto& pos = view.get<PositionComponent>(raw_id);
                const auto& needs = view.get<Population::NeedsComponent>(raw_id);
                const auto& emp = view.get<Population::EmploymentComponent>(raw_id);

                // Ensure PovertyComponent exists
                if (!world.registry.all_of<PovertyComponent>(raw_id)) {
                    world.registry.emplace<PovertyComponent>(raw_id);
                }
                auto& poverty = world.registry.get<PovertyComponent>(raw_id);

                // Condition A: Income Poor (Below 50% of Median)
                bool income_poor = emp.wage.toFloat() < (median_income * POVERTY_INCOME_THRESHOLD);

                // Condition B: Needs Poor (Average Satisfaction < 40)
                float needs_sum = 0.0f;
                for (float sat : needs.satisfaction_levels) {
                    needs_sum += sat;
                }
                float avg_needs = needs_sum / static_cast<float>(Population::NeedType::COUNT);
                bool need_poor = avg_needs < 40.0f;

                // THE WELFARE STATE CHECK: Must be BOTH cash-poor AND need-poor!
                poverty.is_poor = (income_poor && need_poor);

                // Aggregate for instability
                uint32_t region = get_region_for_tile(pos.coord);
                regional_stats[region].total_pop++;
                if (poverty.is_poor) {
                    regional_stats[region].poor_pop++;
                }
            }

            // --- 3. Fire Regional Instability Events ---
            for (const auto& [region_id, stats] : regional_stats) {
                if (stats.total_pop == 0) continue;

                float poverty_rate = static_cast<float>(stats.poor_pop) / static_cast<float>(stats.total_pop);

                // Instability triggers if poverty rate exceeds 20%
                if (poverty_rate > 0.20f) {
                    float instability_delta = poverty_rate * GINI_INSTABILITY_COEFFICIENT;
                    publisher.publish(RegionalInstabilityEvent{
                        region_id,
                        instability_delta,
                        poverty_rate
                    });
                }
            }
        }
    };
}