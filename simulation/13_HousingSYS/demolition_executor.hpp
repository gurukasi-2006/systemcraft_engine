#pragma once

#include <cstdint>
#include <vector>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/entity_id.hpp"

// Required Components
#include "../12_HousingComponent/demolition_candidate_component.hpp"
#include "../12_HousingComponent/occupancy_component.hpp"
#include "../12_HousingComponent/floor_area_density_component.hpp"
#include "../12_HousingComponent/building_identity_component.hpp"
#include "citizen_housing_matcher.hpp" // For Population::HousingComponent

/**
 * @file demolition_executor.hpp
 * @brief Subsystem 163: Safely cycles condemned buildings through a multi-tick teardown pipeline.
 */

namespace Housing {

    // --- Events ---

    struct MaterialSalvagedEvent {
        EntityID building_id;
        float recovered_material_mass;
    };

    /**
     * @class DemolitionExecutor
     * @brief System that processes all active demolitions, evicts tenants incrementally, and safely frees ECS memory.
     */
    class DemolitionExecutor {
    public:
        /**
         * @brief Advances the state machine for all condemned buildings.
         */
        void execute_demolitions(ECSWorld& world, EventPublisher& publisher) {
            auto view = world.registry.view<DemolitionCandidateComponent>();

            // We store entities to destroy AFTER the loop to avoid ECS iterator invalidation
            std::vector<entt::entity> entities_to_destroy;

            for (auto raw : view) {
                auto& demo = view.get<DemolitionCandidateComponent>(raw);
                EntityID bld_id{static_cast<uint32_t>(raw)};

                if (demo.phase == DemolitionPhase::Condemned) {
                    // Stage 1: Issue Notices
                    if (world.registry.all_of<OccupancyComponent>(raw)) {
                        const auto& occ = world.registry.get<OccupancyComponent>(raw);
                        for (auto cit_id : occ.occupant_list) {
                            publisher.publish(EvictionNoticeEvent{bld_id, cit_id, demo.reason, 0});
                        }
                    }
                    demo.phase = DemolitionPhase::EvictingOccupants;
                }
                else if (demo.phase == DemolitionPhase::EvictingOccupants) {
                    // Stage 2: Throttle Evictions (1 per tick)
                    if (world.registry.all_of<OccupancyComponent>(raw)) {
                        auto& occ = world.registry.get<OccupancyComponent>(raw);

                        if (!occ.occupant_list.empty()) {
                            EntityID cit_id = occ.occupant_list.back();
                            occ.occupant_list.pop_back();

                            // Fire event
                            publisher.publish(TenantEvictedEvent{cit_id, demo.reason});

                            // Physically un-house the citizen
                            entt::entity cit_raw = static_cast<entt::entity>(cit_id.raw_id);
                            if (world.registry.valid(cit_raw) && world.registry.all_of<Population::HousingComponent>(cit_raw)) {
                                world.registry.get<Population::HousingComponent>(cit_raw).building_id = EntityID{0};
                            }
                        }

                        // Check if building is finally empty
                        if (occ.occupant_list.empty()) {
                            demo.phase = DemolitionPhase::Clearing;
                        }
                    } else {
                        // No occupancy component at all, skip straight to clearing
                        demo.phase = DemolitionPhase::Clearing;
                    }
                }
                else if (demo.phase == DemolitionPhase::Clearing) {
                    // Stage 3: Material Salvage
                    float salvage_rate = 0.1f; // Fallback
                    float area = 100.0f;       // Fallback

                    if (world.registry.all_of<BuildingIdentityComponent>(raw)) {
                        switch (world.registry.get<BuildingIdentityComponent>(raw).type) {
                            case BuildingType::Shack:      salvage_rate = 0.3f; break;
                            case BuildingType::House:      salvage_rate = 0.5f; break;
                            case BuildingType::LowRiseApt: salvage_rate = 0.6f; break;
                            case BuildingType::Tower:      salvage_rate = 0.4f; break;
                        }
                    }
                    if (world.registry.all_of<FloorAreaDensityComponent>(raw)) {
                        area = world.registry.get<FloorAreaDensityComponent>(raw).gross_floor_area_m2;
                    }

                    publisher.publish(MaterialSalvagedEvent{bld_id, area * salvage_rate});
                    publisher.publish(BuildingDemolishedEvent{bld_id, demo.reason});

                    demo.phase = DemolitionPhase::Done;
                }
                else if (demo.phase == DemolitionPhase::Done) {
                    // Stage 4: ECS Memory Deletion
                    entities_to_destroy.push_back(raw);
                }
            }

            // Safely wipe memory after all systems have processed
            for (auto raw_destroy : entities_to_destroy) {
                world.registry.destroy(raw_destroy);
            }
        }
    };
}