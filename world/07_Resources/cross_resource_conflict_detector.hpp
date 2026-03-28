#pragma once

#include <vector>
#include <entt/entt.hpp>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/resource_type.hpp"
#include "../../core/04_Types/entity_id.hpp"
#include "../../core/03_Event_Bus/immediate_dispatcher.hpp"
#include "deposit_registry.hpp"
#include "resource_deposit_component.hpp"

/**
 * @file cross_resource_conflict_detector.hpp
 * @brief Intercepts construction commands to detect if extracting one resource will destroy another.
 */

namespace Resources {

    /**
     * @struct ResourceConflictEvent
     * @brief Blasted to the UI/Event Bus to halt construction and ask the player for a policy decision.
     */
    struct ResourceConflictEvent {
        TileCoord location;                  ///< Where the conflict is happening
        ResourceType targeted_resource;      ///< What the player WANTS to mine
        ResourceType conflicting_resource;   ///< What the player is going to DESTROY
        EntityID conflicting_entity;         ///< The specific ECS ID of the doomed resource
    };

    /**
     * @class CrossResourceConflictDetector
     * @brief Scans tiles for overlapping, incompatible deposits before approving extraction facilities.
     */
    class CrossResourceConflictDetector {
    private:
        ImmediateDispatcher& dispatcher_;

    public:
        /**
         * @brief Constructs the detector with an immediate event dispatcher.
         */
        explicit CrossResourceConflictDetector(ImmediateDispatcher& dispatcher)
            : dispatcher_(dispatcher) {}

        /**
         * @brief Checks a tile for conflicts. If found, it halts approval and fires an event.
         * @param coord The map tile where the player wants to build.
         * @param targeted_type The resource the proposed facility intends to extract.
         * @param deposit_index The spatial lookup registry.
         * @param ecs The master EnTT registry.
         * @return true if a conflict exists (requires player intervention), false if the tile is clear to build.
         */
        bool detect_conflict(
            TileCoord coord,
            ResourceType targeted_type,
            const DepositRegistry& deposit_index,
            const entt::registry& ecs)
        {
            bool conflict_found = false;
            const auto& deposits = deposit_index.get_deposits(coord);

            for (const EntityID& eid : deposits) {
                entt::entity raw_id = static_cast<entt::entity>(eid.raw_id);

                if (ecs.valid(raw_id) && ecs.all_of<ResourceDepositComponent>(raw_id)) {
                    const auto& comp = ecs.get<ResourceDepositComponent>(raw_id);

                    // A conflict occurs if there is a DIFFERENT resource on this tile that isn't empty
                    if (comp.type != targeted_type && comp.total_reserve_quantity > Fixed32(0.0f)) {

                        // Fire the policy event immediately for the UI to catch
                        dispatcher_.dispatch(ResourceConflictEvent{
                            coord,
                            targeted_type,
                            comp.type,
                            eid
                        });

                        conflict_found = true;
                    }
                }
            }

            return conflict_found;
        }
    };
}