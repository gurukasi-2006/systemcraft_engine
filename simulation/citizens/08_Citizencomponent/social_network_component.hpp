#pragma once

#include <vector>
#include <cstdint>
#include <cmath>
#include <entt/entt.hpp>

#include "../../../core/04_Types/tile_coord.hpp"
#include "../../../world/06_Worldgen/seed_manager.hpp"

/**
 * @file social_network_component.hpp
 * @brief Subsystem 98: Tracks citizen relationships, driving grief contagion, job finding, and political drift.
 */

namespace Population {

    using ClusterID = uint32_t;

    /**
     * @struct AcquaintanceLink
     * @brief A single directional relationship to another simulated citizen.
     */
    struct AcquaintanceLink {
        entt::entity entity;
        float relationship_strength{0.0f}; ///< 0.0 (Strangers) to 1.0 (Family/Spouse)
    };

    /**
     * @struct SocialNetworkComponent
     * @brief ECS Component holding the citizen's social graph and processing connection probabilities.
     */
    struct SocialNetworkComponent {
        ClusterID cluster_id{0}; // Used for macro-level clustering (e.g., the Mass Mortality Grief trigger)

        // Capped at 12 to maintain O(1) performance bounds in dense network queries
        std::vector<AcquaintanceLink> acquaintance_list;

        /**
         * @brief Attempts to form a new connection with a citizen sharing the same tile.
         * @param rng The deterministic random number generator.
         * @param other_citizen The ECS ID of the potential new friend.
         * @param starting_strength The initial bond strength (defaults to low for random encounters).
         * @return True if the connection was successfully formed.
         */
        inline bool try_form_connection(SeedManager& rng, entt::entity other_citizen, float starting_strength = 0.2f) {
            // Hard cap at 12 connections
            if (acquaintance_list.size() >= 12) return false;

            // Prevent self-connections or duplicate links
            for (const auto& link : acquaintance_list) {
                if (link.entity == other_citizen) return false;
            }

            // P(form_link) = 0.0002 * (1 - current_connections / 12.0)
            float capacity_ratio = static_cast<float>(acquaintance_list.size()) / 12.0f;
            float prob_form = 0.0002f * (1.0f - capacity_ratio);

            if (rng.random_fixed(0.0f, 1.0f).toFloat() < prob_form) {
                acquaintance_list.push_back({other_citizen, starting_strength});
                return true;
            }

            return false;
        }

        /**
         * @brief Processes distance-based relationship decay.
         * @param rng The deterministic random number generator.
         * @param my_pos The physical location of this citizen.
         * @param their_pos The physical location of the acquaintance.
         * @param index The specific connection array index to evaluate.
         * @return True if the link degraded and was removed.
         */
        inline bool process_attrition(SeedManager& rng, TileCoord my_pos, TileCoord their_pos, size_t index) {
            if (index >= acquaintance_list.size()) return false;

            // Euclidean distance in tiles
            float dx = static_cast<float>(my_pos.x - their_pos.x);
            float dy = static_cast<float>(my_pos.y - their_pos.y);
            float distance = std::sqrt((dx * dx) + (dy * dy));

            // P(lose_link) = 0.00005 * tile_distance
            float prob_lose = 0.00005f * distance;

            // Optional Realism: Extremely strong bonds (family) resist distance decay
            if (acquaintance_list[index].relationship_strength >= 0.8f) {
                prob_lose *= 0.1f; // 90% resistance to drifting apart
            }

            if (rng.random_fixed(0.0f, 1.0f).toFloat() < prob_lose) {
                // O(1) Swap-and-Pop removal to keep the vector contiguous without shifting memory
                acquaintance_list[index] = acquaintance_list.back();
                acquaintance_list.pop_back();
                return true;
            }

            return false;
        }

        /**
         * @brief Forces a connection without a probability check (used at spawn for family).
         */
        inline void force_connection(entt::entity family_member, float strength = 1.0f) {
            if (acquaintance_list.size() >= 12) return;
            for (const auto& link : acquaintance_list) {
                if (link.entity == family_member) return;
            }
            acquaintance_list.push_back({family_member, strength});
        }
    };
}