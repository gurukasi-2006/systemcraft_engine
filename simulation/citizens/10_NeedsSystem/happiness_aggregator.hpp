#pragma once

#include <entt/entt.hpp>
#include <array>
#include <algorithm>
#include <numeric>

#include "../../../core/01_ECS_core/ecs_world.hpp"

#include "../10_NeedsSystem/need_definitions.hpp" // Subsystem 115
#include "../08_Citizencomponent/happiness_score_component.hpp"

/**
 * @file happiness_aggregator.hpp
 * @brief Subsystem 123: Computes the weighted composite happiness score using exponential smoothing.
 */

namespace Population {

    class HappinessAggregator {
    private:
        // Mathematical weights mapped exactly to the 13 NeedType enums.
        // Sums to exactly 1.00. Emphasizes survival over self-actualization.
        static constexpr std::array<float, static_cast<size_t>(NeedType::COUNT)> NEED_WEIGHTS = {
            0.20f,  // 0: Food (Survival)
            0.10f,  // 1: Water
            0.12f,  // 2: Shelter
            0.10f,  // 3: Healthcare
            0.10f,  // 4: Safety (Stability)
            0.08f,  // 5: Employment
            0.05f,  // 6: Transport
            0.05f,  // 7: Education (Societal)
            0.05f,  // 8: Leisure
            0.04f,  // 9: Community (Systemcraft Unique)
            0.04f,  // 10: Environment
            0.04f,  // 11: ConsumerGoods
            0.03f   // 12: Connectivity
        };

    public:
        /**
         * @brief Calculates raw happiness and applies memory decay (smoothing) for all citizens.
         * @param world The ECS master world.
         */
        void update(ECSWorld& world) {
            auto view = world.registry.view<Population::NeedsComponent, Population::HappinessScoreComponent>();

            for (auto raw_id : view) {
                const auto& needs = view.get<Population::NeedsComponent>(raw_id);
                auto& hap_score = view.get<Population::HappinessScoreComponent>(raw_id);

                // --- 1. Compute Raw Instantaneous Happiness ---
                float raw_happiness = 0.0f;

                // Dot product of current satisfaction and their ideological/biological weights
                for (size_t i = 0; i < static_cast<size_t>(NeedType::COUNT); ++i) {
                    raw_happiness += needs.satisfaction_levels[i] * NEED_WEIGHTS[i];
                }

                // Safety clamp
                raw_happiness = std::clamp(raw_happiness, 0.0f, 100.0f);

                // --- 2. Exponential Smoothing (Psychological Memory) ---
                // alpha = 0.08. 8% of their mood is today's reality, 92% is their recent memory/grudges.
                float smoothed_happiness = (0.08f * raw_happiness) + (0.92f * hap_score.current_happiness);

                // Store back to component
                hap_score.current_happiness = std::clamp(smoothed_happiness, 0.0f, 100.0f);
            }
        }
    };
}