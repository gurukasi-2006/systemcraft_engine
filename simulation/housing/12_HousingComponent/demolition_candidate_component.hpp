#pragma once

#include <cstdint>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/entity_id.hpp"
#include "../../../core/04_Types/time_constants.hpp"

#include "occupancy_component.hpp" // To access the tenant list

/**
 * @file demolition_candidate_component.hpp
 * @brief Subsystem 152: State machine for safe, staged building demolition and eviction.
 */

namespace Housing {

    /**
     * @enum DemolitionReason
     * @brief The trigger condition for the bulldozer.
     */
    enum class DemolitionReason : uint8_t {
        Derelict = 0,
        Rezoning,
        DisasterDamage,
        PlayerOrder,
        NonCompliance
    };

    /**
     * @enum DemolitionPhase
     * @brief Staged pipeline to prevent instantaneous ECS entity destruction.
     */
    enum class DemolitionPhase : uint8_t {
        Condemned = 0,      // 0. Just tagged, needs eviction notices sent
        EvictingOccupants,  // 1. Waiting for the 30-day notice to expire
        Clearing,           // 2. Physical tear-down (materials, spatial index)
        Done                // 3. Safe to destroy ECS entity memory
    };

    // --- Demolition Events ---

    struct EvictionNoticeEvent {
        EntityID building_id;
        EntityID citizen_id;
        DemolitionReason reason;
        uint64_t move_out_by_tick;
    };

    struct TenantEvictedEvent {
        EntityID citizen_id;
        DemolitionReason reason;
    };

    struct BuildingDemolishedEvent {
        EntityID building_id;
        DemolitionReason reason;
    };

    /**
     * @struct DemolitionCandidateComponent
     * @brief Attached to a building to schedule its removal.
     */
    struct DemolitionCandidateComponent {
        DemolitionReason reason{DemolitionReason::PlayerOrder};
        DemolitionPhase phase{DemolitionPhase::Condemned};
        uint64_t target_tick{0};

        /**
         * @brief Advances the demolition state machine based on simulation time.
         * @param world The ECS World to read/modify occupants.
         * @param publisher Event bus for eviction and demolition alerts.
         * @param self_id This building's ID.
         * @param current_tick Absolute simulation time.
         */
        void process_demolition(ECSWorld& world, EventPublisher& publisher, EntityID self_id, uint64_t current_tick) {
            entt::entity raw = static_cast<entt::entity>(self_id.raw_id);
            if (!world.registry.valid(raw)) return;

            if (phase == DemolitionPhase::Condemned) {
                // Phase 1: Issue 30-day eviction notices
                target_tick = current_tick + (TimeConstants::TICKS_PER_DAY * 30);

                if (world.registry.all_of<OccupancyComponent>(raw)) {
                    auto& occ = world.registry.get<OccupancyComponent>(raw);
                    for (auto cit_id : occ.occupant_list) {
                        publisher.publish(EvictionNoticeEvent{self_id, cit_id, reason, target_tick});
                    }
                }

                phase = DemolitionPhase::EvictingOccupants;
            }
            else if (phase == DemolitionPhase::EvictingOccupants) {
                // Phase 2: Wait for 30 days to expire, then force them out
                if (current_tick >= target_tick) {
                    if (world.registry.all_of<OccupancyComponent>(raw)) {
                        auto& occ = world.registry.get<OccupancyComponent>(raw);

                        // Force out remaining tenants into the homeless queue
                        for (auto cit_id : occ.occupant_list) {
                            publisher.publish(TenantEvictedEvent{cit_id, reason});
                        }
                        occ.occupant_list.clear();
                    }

                    // Allow 7 days for physical clearing and materials salvaging
                    target_tick = current_tick + (TimeConstants::TICKS_PER_DAY * 7);
                    phase = DemolitionPhase::Clearing;
                }
            }
            else if (phase == DemolitionPhase::Clearing) {
                // Phase 3: Wait for teardown to finish, mark for ECS deletion
                if (current_tick >= target_tick) {
                    publisher.publish(BuildingDemolishedEvent{self_id, reason});
                    phase = DemolitionPhase::Done;
                }
            }
            else if (phase == DemolitionPhase::Done) {
                // Phase 4: Awaiting the master housing system to call world.entity_manager.destroyEntity()
                // We do nothing here to ensure safety.
            }
        }
    };
}