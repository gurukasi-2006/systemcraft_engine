#pragma once

#include <entt/entt.hpp>
#include <unordered_map>
#include <vector>
#include <cmath>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/time_constants.hpp"

#include "../08_Citizencomponent/political_belief_component.hpp" // Has social_axis and econ_axis

/**
 * @file ethnic_cultural_group_tracker.hpp
 * @brief Subsystem 136: Tracks cultural group political alignment and overall social cohesion.
 */

namespace Demographics {

    /**
     * @struct CulturalIdentityComponent
     * @brief Optional component mapping a citizen to an ethnic, religious, or cultural group.
     */
    struct CulturalIdentityComponent {
        uint32_t group_id{0};
    };

    /**
     * @struct CulturalDemographicsEvent
     * @brief Monthly macroeconomic packet containing group political leans and social cohesion.
     */
    struct CulturalDemographicsEvent {
        std::unordered_map<uint32_t, float> group_political_lean;
        float overall_cohesion{1.0f};
    };

    /**
     * @struct SocialTensionWarningEvent
     * @brief Fired when societal polarization between cultural groups becomes dangerously high.
     */
    struct SocialTensionWarningEvent {
        float cohesion_level;
    };

    class EthnicCulturalGroupTracker {
    public:
        /**
         * @brief Scans cultural groups, calculates polarization (stddev), and fires warnings.
         * @param world ECS World.
         * @param publisher Event Bus.
         * @param current_tick Absolute simulation time.
         * @param cultural_groups_enabled The Scenario Flag controlling this feature.
         */
        void update(ECSWorld& world, EventPublisher& publisher, uint64_t current_tick, bool cultural_groups_enabled = true) {

            // Feature Gate: Only run if the scenario explicitly allows ethnic/cultural tracking
            if (!cultural_groups_enabled) {
                return;
            }

            constexpr uint64_t MONTH_TICKS = TimeConstants::TICKS_PER_YEAR / 12ULL;

            // Execute only on the monthly rollover tick
            if (current_tick == 0 || current_tick % MONTH_TICKS != 0) {
                return;
            }

            auto view = world.registry.view<CulturalIdentityComponent, Population::PoliticalBeliefComponent>();
            //if (view.empty()) return;

            std::unordered_map<uint32_t, float> group_econ_sum;
            std::unordered_map<uint32_t, uint32_t> group_count;

            // --- 1. Accumulate Political Leanings by Group ---
            for (auto raw_id : view) {
                const auto& cult = view.get<CulturalIdentityComponent>(raw_id);
                const auto& pol = view.get<Population::PoliticalBeliefComponent>(raw_id);

                group_econ_sum[cult.group_id] += pol.economic_axis;
                group_count[cult.group_id]++;
            }

            CulturalDemographicsEvent report;
            std::vector<float> group_means;

            // --- 2. Calculate Group Means ---
            for (const auto& [grp, count] : group_count) {
                float mean = group_econ_sum[grp] / static_cast<float>(count);
                report.group_political_lean[grp] = mean;
                group_means.push_back(mean);
            }

            // --- 3. Calculate Cohesion via Standard Deviation ---
            float cohesion = 1.0f;

            if (group_means.size() > 1) {
                float mean_of_means = 0.0f;
                for (float m : group_means) {
                    mean_of_means += m;
                }
                mean_of_means /= static_cast<float>(group_means.size());

                float variance_sum = 0.0f;
                for (float m : group_means) {
                    variance_sum += (m - mean_of_means) * (m - mean_of_means);
                }

                float variance = variance_sum / static_cast<float>(group_means.size());
                float stddev = std::sqrt(variance);

                // Formula: 1.0 - (stddev / 2.0)
                cohesion = 1.0f - (stddev / 2.0f);
            }

            report.overall_cohesion = cohesion;
            publisher.publish(report);

            // --- 4. Fire Social Tension Warning ---
            if (cohesion < 0.4f) {
                publisher.publish(SocialTensionWarningEvent{cohesion});
            }
        }
    };
}