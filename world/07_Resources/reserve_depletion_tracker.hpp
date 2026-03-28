#pragma once

#include <cstdint>
#include <entt/entt.hpp>

#include "../../core/04_Types/entity_id.hpp"
#include "../../core/04_Types/fixed_point.hpp"
#include "../../core/03_Event_Bus/immediate_dispatcher.hpp"
#include "resource_deposit_component.hpp"

/**
 * @file reserve_depletion_tracker.hpp
 * @brief Safely reduces physical deposit reserves and dispatches global events when thresholds are crossed.
 */

namespace Resources {

    /**
     * @struct DepositLowEvent
     * @brief Fired when a deposit's reserves drop below the designated warning threshold.
     */
    struct DepositLowEvent {
        EntityID deposit_entity;
        ResourceType type;
        Fixed32 remaining_quantity;
    };

    /**
     * @struct DepositEmptyEvent
     * @brief Fired the exact tick a deposit hits 0.0 reserves and becomes exhausted.
     */
    struct DepositEmptyEvent {
        EntityID deposit_entity;
        ResourceType type;
    };

    /**
     * @class ReserveDepletionTracker
     * @brief Modifies the ECS state for mined deposits and handles threshold logic.
     */
    class ReserveDepletionTracker {
    private:
        ImmediateDispatcher& dispatcher_;
        Fixed32 low_threshold_;

    public:
        /**
         * @brief Constructs the tracker with an event dispatcher and a low-warning threshold.
         * @param dispatcher The real-time event router.
         * @param low_threshold The value at which to fire the low warning (default 1000.0).
         */
        explicit ReserveDepletionTracker(ImmediateDispatcher& dispatcher, Fixed32 low_threshold = Fixed32(1000.0f))
            : dispatcher_(dispatcher), low_threshold_(low_threshold) {}

        /**
         * @brief Deducts the extracted amount from the entity's component and checks triggers.
         * @param reg The master EnTT registry.
         * @param deposit_entity The exact ECS ID of the resource node being mined.
         * @param amount_extracted The yield calculated by the ExtractionRateCalculator.
         */
        void deduct_extraction(entt::registry& reg, EntityID deposit_entity, Fixed32 amount_extracted) {
            entt::entity raw_id = static_cast<entt::entity>(deposit_entity.raw_id);

            // Safety Check: Ensure the entity exists and actually has a deposit component
            if (!reg.valid(raw_id) || !reg.all_of<ResourceDepositComponent>(raw_id)) {
                return;
            }

            auto& deposit = reg.get<ResourceDepositComponent>(raw_id);

            if (deposit.total_reserve_quantity <= Fixed32(0.0f)) {
                return; // Already exhausted, prevent redundant math
            }

            Fixed32 previous_qty = deposit.total_reserve_quantity;
            deposit.total_reserve_quantity = deposit.total_reserve_quantity - amount_extracted;

            // Enforce the physical floor of reality
            if (deposit.total_reserve_quantity < Fixed32(0.0f)) {
                deposit.total_reserve_quantity = Fixed32(0.0f);
            }

            // --- Event Dispatching Logic ---
            // 1. Check if it just crossed the low threshold this specific tick
            if (previous_qty > low_threshold_ && deposit.total_reserve_quantity <= low_threshold_) {
                dispatcher_.dispatch(DepositLowEvent{deposit_entity, deposit.type, deposit.total_reserve_quantity});
            }

            // 2. Check if it just became completely empty this specific tick
            if (previous_qty > Fixed32(0.0f) && deposit.total_reserve_quantity == Fixed32(0.0f)) {
                dispatcher_.dispatch(DepositEmptyEvent{deposit_entity, deposit.type});
            }
        }
    };
}