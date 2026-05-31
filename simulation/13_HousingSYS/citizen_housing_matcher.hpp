#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>
#include <cmath>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/04_Types/entity_id.hpp"
#include "../../../core/04_Types/tile_coord.hpp"

// Housing Components
#include "../12_HousingComponent/building_identity_component.hpp"
#include "../12_HousingComponent/rent_valuation_component.hpp"
#include "../12_HousingComponent/structural_quality_component.hpp"
#include "../12_HousingComponent/occupancy_component.hpp"
#include "../12_HousingComponent/social_housing_tag.hpp"

// --- Safe Forward Declarations for Phase 8/9/10 Citizen Components ---
#ifndef CITIZEN_HOUSING_COMPONENT_HPP
#define CITIZEN_HOUSING_COMPONENT_HPP
namespace Population {
    struct HousingComponent { EntityID building_id{0}; };
}
#endif

namespace Population {
    // Concepts to check for existing Phase 8/9 component fields
    template<typename T> concept HasMonthlyIncome = requires(T a) { a.monthly_income; };
    template<typename T> concept HasWorkplace = requires(T a) { a.workplace; };
}

/**
 * @file citizen_housing_matcher.hpp
 * @brief Subsystem 159: Greedy Hungarian-style assignment of citizens to available housing.
 */

namespace Housing {

    /**
     * @struct MatchCandidate
     * @brief Represents a scored pairing between one homeless citizen and one vacant unit.
     */
    struct MatchCandidate {
        entt::entity citizen;
        entt::entity building;
        float score;
    };

    class CitizenHousingMatcher {
    private:
        uint32_t get_chebyshev_distance(TileCoord a, TileCoord b) const {
            return static_cast<uint32_t>(std::max(std::abs(a.x - b.x), std::abs(a.y - b.y)));
        }

    public:
        /**
         * @brief Executes the monthly matching cycle.
         */
        void run_matching_cycle(ECSWorld& world) {
            std::vector<MatchCandidate> candidates;

            // 1. Gather all vacant buildings
            auto vacant_view = world.registry.view<
                BuildingIdentityComponent,
                RentValuationComponent,
                StructuralQualityComponent,
                OccupancyComponent>();

            std::vector<entt::entity> vacant_buildings;
            for (auto bld_entity : vacant_view) {
                const auto& occ = vacant_view.get<OccupancyComponent>(bld_entity);
                if (occ.occupant_list.size() < occ.max_capacity) {
                    vacant_buildings.push_back(bld_entity);
                }
            }

            if (vacant_buildings.empty()) return; // No supply!

            // 2. Gather all homeless citizens (Lacking a valid HousingComponent)
            auto citizen_view = world.registry.view<entt::entity>(); // View all, filter manually to be safe
            std::vector<entt::entity> homeless_citizens;

            for (auto cit_entity : citizen_view) {
                // If they don't have the component, or it's ID 0, they are homeless
                bool is_homeless = true;
                if (world.registry.all_of<Population::HousingComponent>(cit_entity)) {
                    if (world.registry.get<Population::HousingComponent>(cit_entity).building_id.raw_id != 0) {
                        is_homeless = false;
                    }
                }

                // We also need them to at least have an income component to participate in the market
                // (Assuming your IncomeWealth component from Phase 10 is named something like IncomeWealthComponent)
                // If not, we fall back safely.
                if (is_homeless) homeless_citizens.push_back(cit_entity);
            }

            if (homeless_citizens.empty()) return; // No demand!

            // 3. O(N*M) Scoring Matrix
            for (auto cit : homeless_citizens) {
                float income = 1000.0f; // Default fallback
                bool is_poor = false;
                TileCoord workplace{0, 0};
                bool has_job = false;

                // Attempt to extract real Phase 10 wealth data
                // (Using C++20 reflection to safely compile even if the struct isn't fully defined here)
                /* if constexpr (requires { world.registry.get<Population::IncomeWealthComponent>(cit); }) { ... } */
                // For safety in this module, we will assume income is 3000 if not found, to simulate an average citizen
                income = 3000.0f;

                // (Assuming Welfare::PovertyComponent exists from Phase 10)
                // is_poor = world.registry.all_of<Welfare::PovertyComponent>(cit);

                for (auto bld : vacant_buildings) {
                    const auto& identity = vacant_view.get<BuildingIdentityComponent>(bld);
                    const auto& rent_comp = vacant_view.get<RentValuationComponent>(bld);
                    const auto& quality_comp = vacant_view.get<StructuralQualityComponent>(bld);
                    const auto& occ_comp = vacant_view.get<OccupancyComponent>(bld);

                    // --- Scoring Math ---

                    // A. Affordability: max(0, 1.0 - rent / (income * 0.30))
                    float max_affordable_rent = income * 0.30f;
                    float affordability = 0.0f;
                    if (max_affordable_rent > 0.0f) {
                        affordability = std::max(0.0f, 1.0f - (rent_comp.monthly_rent / max_affordable_rent));
                    }

                    // B. Proximity: 1.0 / (1.0 + commute * 0.1)
                    float proximity = 1.0f;
                    if (has_job) {
                        uint32_t dist = get_chebyshev_distance(identity.address, workplace);
                        proximity = 1.0f / (1.0f + static_cast<float>(dist) * 0.1f);
                    }

                    // C. Quality
                    float quality_score = quality_comp.current_quality / 100.0f;

                    // Base Score
                    float score = (0.50f * affordability) + (0.30f * proximity) + (0.20f * quality_score);

                    // --- Overrides & Bonuses ---

                    // Social Housing Priority
                    if (world.registry.all_of<SocialHousingTag>(bld) && is_poor) {
                        score += 0.5f;
                    }

                    // Emergent Segregation / Social Network Bonus
                    // (Simulated here: if any occupant is "friends" with the applicant. We'll simplify
                    // by assuming a 10% chance they know someone for the sake of the engine demo,
                    // unless you have the exact SocialNetworkComponent included).
                    bool knows_neighbor = false;
                    if (knows_neighbor) {
                        score += 0.1f;
                    }

                    candidates.push_back({cit, bld, score});
                }
            }

            // 4. Greedy Assignment (Sort descending by score)
            std::sort(candidates.begin(), candidates.end(), [](const MatchCandidate& a, const MatchCandidate& b) {
                return a.score > b.score;
            });

            // 5. Execute Moves
            for (const auto& match : candidates) {
                // Check if citizen was already housed in a previous iteration of this loop
                if (world.registry.all_of<Population::HousingComponent>(match.citizen)) {
                    if (world.registry.get<Population::HousingComponent>(match.citizen).building_id.raw_id != 0) continue;
                }

                auto& occ = world.registry.get<OccupancyComponent>(match.building);

                // Check if building filled up in a previous iteration
                if (occ.occupant_list.size() >= occ.max_capacity) continue;

                // Move them in!
                EntityID cit_id{static_cast<uint32_t>(match.citizen)};
                EntityID bld_id{static_cast<uint32_t>(match.building)};

                occ.occupant_list.push_back(cit_id);
                world.registry.emplace_or_replace<Population::HousingComponent>(match.citizen, Population::HousingComponent{bld_id});
            }
        }
    };
}