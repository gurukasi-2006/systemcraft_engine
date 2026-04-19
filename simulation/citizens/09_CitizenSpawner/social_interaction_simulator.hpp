#pragma once

#include <entt/entt.hpp>
#include <random>
#include <algorithm>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../08_Citizencomponent/social_network_component.hpp"
#include "../08_Citizencomponent/political_belief_component.hpp"
#include "../08_Citizencomponent/migration_intent_component.hpp"
#include "../08_Citizencomponent/happiness_score_component.hpp"

/**
 * @file social_interaction_simulator.hpp
 * @brief Subsystem 111: Simulates peer influence and social contagion at O(k) cost.
 * @details Iterates over the pre-cached social network to align beliefs and cascade migration intent.
 */

class SocialInteractionSimulator {
public:
    /**
     * @brief Periodically samples acquaintances and aligns beliefs and intent.
     * @param world The ECS master world.
     * @param rng Deterministic random number generator.
     */
    void update(ECSWorld& world, std::mt19937& rng) {
        auto view = world.registry.view<
            Population::SocialNetworkComponent,
            Population::PoliticalBeliefComponent,
            Population::MigrationIntentComponent,
            Population::HappinessScoreComponent
        >();

        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        for (auto raw_id : view) {
            auto& soc_a = view.get<Population::SocialNetworkComponent>(raw_id);
            auto& pol_a = view.get<Population::PoliticalBeliefComponent>(raw_id);
            auto& mig_a = view.get<Population::MigrationIntentComponent>(raw_id);
            auto& hap_a = view.get<Population::HappinessScoreComponent>(raw_id);

            // Iterate over the fixed-size social graph (O(k) complexity)
            for (const auto& link : soc_a.acquaintance_list) {

                // 1% chance per tick per acquaintance pair to interact
                if (dist(rng) >= 0.01f) {
                    continue;
                }

                entt::entity friend_id = link.entity;

                // Ensure the friend still exists in the ECS and hasn't died/migrated
                if (!world.registry.valid(friend_id) ||
                    !world.registry.all_of<Population::PoliticalBeliefComponent,
                                           Population::MigrationIntentComponent,
                                           Population::HappinessScoreComponent>(friend_id)) {
                    continue;
                }

                auto& pol_b = world.registry.get<Population::PoliticalBeliefComponent>(friend_id);
                auto& mig_b = world.registry.get<Population::MigrationIntentComponent>(friend_id);
                auto& hap_b = world.registry.get<Population::HappinessScoreComponent>(friend_id);

                // Influence multiplier based on relationship strength
                float influence_weight = 1.0f + (link.relationship_strength - 0.5f) * 0.4f;

                // --- 1. Belief Interpolation ---
                pol_a.economic_axis += (pol_b.economic_axis - pol_a.economic_axis) * 0.001f * influence_weight;
                pol_a.social_axis += (pol_b.social_axis - pol_a.social_axis) * 0.001f * influence_weight;

                // Clamp axes mathematically to prevent runaway extremes
                pol_a.economic_axis = std::clamp(pol_a.economic_axis, -1.0f, 1.0f);
                pol_a.social_axis = std::clamp(pol_a.social_axis, -1.0f, 1.0f);

                // --- 2. Mood Contagion ---
                hap_a.current_happiness += (hap_b.current_happiness - hap_a.current_happiness) * 0.01f * influence_weight;
                hap_a.current_happiness = std::clamp(hap_a.current_happiness, 0.0f, 100.0f);

                // --- 3. Migration Intent Contagion (Chain Migration) ---
                if (mig_b.intent_score > 0.8f) {
                    mig_a.intent_score += 0.005f;
                    mig_a.intent_score = std::clamp(mig_a.intent_score, 0.0f, 3.0f);
                }
            }
        }
    }
};