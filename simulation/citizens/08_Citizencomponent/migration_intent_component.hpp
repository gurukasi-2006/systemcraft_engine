#pragma once

#include <cstdint>
#include <algorithm>
#include <optional>
#include <vector>

/**
 * @file migration_intent_component.hpp
 * @brief Subsystem 99: Tracks the push/pull factors of relocation and calculates target regions.
 */

namespace Population {

    using RegionID = uint32_t;

    /**
     * @struct RegionAttractivenessData
     * @brief A data packet provided by the global spatial system to evaluate potential new homes.
     */
    struct RegionAttractivenessData {
        RegionID id;
        float job_vacancy_rate;     ///< Normalized rate, e.g., 0.0 to 100.0
        float housing_availability; ///< Normalized rate, e.g., 0.0 to 100.0
        float safety_index;         ///< 0.0 to 100.0
        uint32_t social_connections;///< Count of the citizen's acquaintances living in this region
    };

    /**
     * @struct MigrationIntentComponent
     * @brief ECS Component acting as a "pressure gauge" for citizen relocation.
     */
    struct MigrationIntentComponent {
        float intent_score{0.0f};   ///< Ranges from 0.0 (Settled) to 3.0 (Desperate)
        std::optional<RegionID> target_region{std::nullopt};

        /**
         * @brief Evaluates current living conditions and adjusts the migration pressure gauge.
         * @param is_unemployed True if the citizen lacks a job.
         * @param housing_quality Score from 0.0 to 100.0.
         * @param safety_satisfaction Score from 0.0 to 100.0.
         * @param food_satisfaction Score from 0.0 to 100.0.
         * @param happiness Overall aggregate happiness from 0.0 to 100.0.
         * @param social_connections_count How many friends/family they have locally.
         */
        inline void update_intent(bool is_unemployed, float housing_quality,
                                  float safety_satisfaction, float food_satisfaction,
                                  float happiness, uint32_t social_connections_count)
        {
            float delta = 0.0f;

            // Push Factors (Negative local conditions)
            if (is_unemployed) delta += 0.003f;
            if (housing_quality < 30.0f) delta += 0.005f;
            if (safety_satisfaction < 20.0f) delta += 0.008f;
            if (food_satisfaction < 40.0f) delta += 0.002f;

            // Pull/Anchor Factors (Reasons to stay)
            if (happiness > 60.0f) delta -= 0.002f;
            if (social_connections_count >= 6) delta -= 0.003f;

            // Apply delta and clamp to the 0.0 - 3.0 bound
            intent_score = std::clamp(intent_score + delta, 0.0f, 3.0f);
        }

        /**
         * @brief Checks if the citizen's patience has boiled over.
         * @return True if the score has crossed the critical 1.0 threshold.
         */
        inline bool is_ready_to_migrate() const {
            return intent_score >= 1.0f;
        }

        /**
         * @brief Scans available regions and locks in a destination based on the player's weighted formula.
         * @param candidate_regions A list of valid regions supplied by the migration system.
         */
        inline void select_target_region(const std::vector<RegionAttractivenessData>& candidate_regions) {
            float best_score = -1.0f;
            std::optional<RegionID> best_region = std::nullopt;

            for (const auto& region : candidate_regions) {
                // The weighted pull formula
                float score = (0.4f * region.job_vacancy_rate) +
                              (0.3f * region.housing_availability) +
                              (0.2f * region.safety_index) +
                              (0.1f * static_cast<float>(region.social_connections));

                if (score > best_score) {
                    best_score = score;
                    best_region = region.id;
                }
            }

            target_region = best_region;
        }

        /**
         * @brief Called after a successful move to settle the citizen down in their new home.
         */
        inline void reset_intent() {
            intent_score = 0.0f;
            target_region = std::nullopt;
        }
    };
}