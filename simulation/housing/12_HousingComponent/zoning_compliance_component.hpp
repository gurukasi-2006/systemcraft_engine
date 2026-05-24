#pragma once

#include <cstdint>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/entity_id.hpp"

#include "rent_valuation_component.hpp" // For market valuation
#include "demolition_candidate_component.hpp" // For the real state machine

/**
 * @file zoning_compliance_component.hpp
 * @brief Subsystem 145: Enforces zoning laws, issues escalating fines, and tags illegal buildings for demolition.
 */

namespace Housing {

    /**
     * @enum ZoneType
     * @brief The designated urban planning district.
     */
    enum class ZoneType : uint8_t {
        ResLow = 0,
        ResHigh,
        Commercial,
        Industrial,
        Mixed,
        Protected,
        COUNT
    };

    /**
     * @struct ZoningFineEvent
     * @brief Fired monthly to deduct punitive damages from the building owner's finances.
     */
    struct ZoningFineEvent {
        EntityID building_id;
        float fine_amount;
    };



    /**
     * @struct ZoningComplianceComponent
     * @brief Tracks adherence to local zoning laws and calculates escalating penalties.
     */
    struct ZoningComplianceComponent {
        ZoneType zone_type{ZoneType::ResLow};
        bool is_compliant{true};
        uint32_t months_non_compliant{0};

        /**
         * @brief Evaluates compliance, assesses fines, and orders demolitions if ignored for a year.
         * @param world The ECS World to query valuation and attach tags.
         * @param publisher The Event Bus to fire fine events.
         * @param self_id The EntityID of this building.
         */
        void process_compliance(ECSWorld& world, EventPublisher& publisher, EntityID self_id) {
            entt::entity raw = static_cast<entt::entity>(self_id.raw_id);
            if (!world.registry.valid(raw)) return;

            // 1. Safe Harbor
            if (is_compliant) {
                months_non_compliant = 0;
                return;
            }

            // 2. Escalate Non-Compliance
            months_non_compliant++;

            // Extract market valuation (fallback to 0 if not yet assessed)
            float market_val = 0.0f;
            if (world.registry.all_of<RentValuationComponent>(raw)) {
                market_val = world.registry.get<RentValuationComponent>(raw).market_valuation;
            }

            // 3. Calculate Escalating Fine
            // Base fine: 0.5% of market value
            float base_fine = market_val * 0.005f;

            // Modifier: +10% for every month non-compliant
            float modifier = 1.0f + (static_cast<float>(months_non_compliant) * 0.1f);
            float monthly_fine = base_fine * modifier;

            // Dispatch the fine to the economy system
            if (monthly_fine > 0.0f) {
                publisher.publish(ZoningFineEvent{self_id, monthly_fine});
            }

            // 4. The Final Warning (12 Months)
            if (months_non_compliant >= 12) {
                world.registry.emplace_or_replace<DemolitionCandidateComponent>(raw,
                    DemolitionCandidateComponent{DemolitionReason::NonCompliance}
                );
            }
        }
    };
}