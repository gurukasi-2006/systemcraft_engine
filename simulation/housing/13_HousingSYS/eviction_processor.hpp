#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>
#include <unordered_map>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/entity_id.hpp"

// Housing Components
#include "../12_HousingComponent/rent_valuation_component.hpp"
#include "../12_HousingComponent/occupancy_component.hpp"
#include "citizen_housing_matcher.hpp" // For Population::HousingComponent

/**
 * @file eviction_processor.hpp
 * @brief Subsystem 160: Handles monthly rent collection, arrears forgiveness, and mass eviction cascades.
 */

// Safe fallback for Phase 10 Wealth Component
#ifndef POPULATION_WEALTH_COMPONENT_HPP
#define POPULATION_WEALTH_COMPONENT_HPP
namespace Population {
    struct WealthComponent {
        float savings{0.0f};
    };
}
#endif

namespace Housing {

    // --- Events ---

    struct RentArrearsEvent {
        EntityID citizen_id;
        EntityID building_id;
        uint8_t arrears_count;
    };

    struct RentEvictionEvent {
        EntityID citizen_id;
        EntityID building_id;
    };

    struct HomelessnessSurgeEvent {
        uint32_t total_evicted_this_month;
        float highest_rent_spike_pct;
    };

    /**
     * @struct TenantStateComponent
     * @brief Attached to citizens to track their standing with their landlord.
     */
    struct TenantStateComponent {
        uint8_t arrears_counter{0};
    };

    /**
     * @class EvictionProcessor
     * @brief Processes monthly rent affordability and executes evictions.
     */
    class EvictionProcessor {
    private:
        // Tracks the rent of buildings from the previous month to detect macroeconomic shocks (>15% spikes)
        std::unordered_map<uint32_t, float> previous_month_rent_;

    public:
        /**
         * @brief Runs the monthly rent collection cycle across the city.
         */
        void process_monthly_rent(ECSWorld& world, EventPublisher& publisher) {
            auto view = world.registry.view<RentValuationComponent, OccupancyComponent>();

            uint32_t city_wide_evictions = 0;
            float max_spike_pct = 0.0f;

            for (auto raw_bld : view) {
                const auto& rent_comp = view.get<RentValuationComponent>(raw_bld);
                auto& occ_comp = view.get<OccupancyComponent>(raw_bld);
                EntityID bld_id{static_cast<uint32_t>(raw_bld)};

                // 1. Detect Rent Spikes (Demand Shock)
                float prev_rent = rent_comp.monthly_rent;
                if (previous_month_rent_.find(bld_id.raw_id) != previous_month_rent_.end()) {
                    prev_rent = previous_month_rent_[bld_id.raw_id];
                }

                float rent_increase_pct = 0.0f;
                if (prev_rent > 0.0f) {
                    rent_increase_pct = (rent_comp.monthly_rent - prev_rent) / prev_rent;
                }
                if (rent_increase_pct > max_spike_pct) {
                    max_spike_pct = rent_increase_pct;
                }

                // Update ledger for next month
                previous_month_rent_[bld_id.raw_id] = rent_comp.monthly_rent;

                // 2. Process Occupants
                for (auto it = occ_comp.occupant_list.begin(); it != occ_comp.occupant_list.end(); ) {
                    EntityID cit_id = *it;
                    entt::entity raw_cit = static_cast<entt::entity>(cit_id.raw_id);

                    // Ensure citizen has state
                    if (!world.registry.all_of<TenantStateComponent>(raw_cit)) {
                        world.registry.emplace<TenantStateComponent>(raw_cit);
                    }
                    if (!world.registry.all_of<Population::WealthComponent>(raw_cit)) {
                        world.registry.emplace<Population::WealthComponent>(raw_cit);
                    }

                    auto& tenant = world.registry.get<TenantStateComponent>(raw_cit);
                    auto& wealth = world.registry.get<Population::WealthComponent>(raw_cit);

                    // 3. Affordability Check
                    if (wealth.savings < rent_comp.monthly_rent) {
                        tenant.arrears_counter++;
                        publisher.publish(RentArrearsEvent{cit_id, bld_id, tenant.arrears_counter});

                        // 4. Eviction Trigger (3 strikes)
                        if (tenant.arrears_counter >= 3) {
                            publisher.publish(RentEvictionEvent{cit_id, bld_id});

                            // Make Homeless
                            if (world.registry.all_of<Population::HousingComponent>(raw_cit)) {
                                world.registry.get<Population::HousingComponent>(raw_cit).building_id = EntityID{0};
                            }
                            tenant.arrears_counter = 0; // Debt wiped upon eviction
                            city_wide_evictions++;

                            // Erase from building and advance iterator safely
                            it = occ_comp.occupant_list.erase(it);
                            continue;
                        }
                    } else {
                        // 5. Arrears Forgiveness & Payment
                        wealth.savings -= rent_comp.monthly_rent; // Pay rent
                        if (tenant.arrears_counter > 0) {
                            tenant.arrears_counter--; // One good month buys forgiveness
                        }
                    }
                    ++it;
                }
            }

            // 6. Macroeconomic Cascade Check
            // If rent spiked > 15% anywhere in the city and it caused mass evictions (> 10 citizens)
            if (max_spike_pct > 0.15f && city_wide_evictions >= 10) {
                publisher.publish(HomelessnessSurgeEvent{city_wide_evictions, max_spike_pct});
            }
        }
    };
}