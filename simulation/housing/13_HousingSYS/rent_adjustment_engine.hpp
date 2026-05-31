#pragma once

#include <cstdint>
#include <algorithm>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/entity_id.hpp"

// Required Components
#include "../12_HousingComponent/rent_valuation_component.hpp"
#include "../12_HousingComponent/structural_quality_component.hpp"
#include "../12_HousingComponent/social_housing_tag.hpp"

/**
 * @file rent_adjustment_engine.hpp
 * @brief Subsystem 161: Monthly rent price updates utilizing supply/demand bounds and policy caps.
 */

namespace Housing {

    /**
     * @struct PropertyFinancesComponent
     * @brief Supplemental data for private properties tracking operating costs and landlord behavior.
     */
    struct PropertyFinancesComponent {
        float monthly_maintenance_cost{100.0f};
        float loan_repayment{400.0f};
        float risk_appetite{0.5f}; // 0.0 (Conservative) to 1.0 (Aggressive/Speculative)
    };

    // --- Regulatory Events ---

    struct RentControlFineEvent {
        EntityID building_id;
        float attempted_rent;
        float capped_rent;
    };

    /**
     * @class RentAdjustmentEngine
     * @brief Processes market dynamics and adjusts rent across the private market.
     */
    class RentAdjustmentEngine {
    public:
        /**
         * @brief Recalculates rent for all private market housing.
         * @param regional_vacancy_rate The current percentage of empty homes in the zone.
         * @param median_income City-wide median income (used to calculate caps).
         * @param rent_control_active True if the government has enacted cap policies.
         */
        void process_monthly_adjustments(ECSWorld& world, EventPublisher& publisher,
                                         float regional_vacancy_rate, float median_income,
                                         bool rent_control_active) {

            // Social Housing is explicitly EXEMPT from market rates via the exclude_t filter
            auto view = world.registry.view<RentValuationComponent, StructuralQualityComponent, PropertyFinancesComponent>(entt::exclude<SocialHousingTag>);

            float natural_vac = 0.05f;
            float demand_pressure = (natural_vac - regional_vacancy_rate) / natural_vac;
            float rent_cap = median_income * 0.35f;

            for (auto raw : view) {
                auto& rent = view.get<RentValuationComponent>(raw);
                const auto& quality = view.get<StructuralQualityComponent>(raw);
                const auto& finances = view.get<PropertyFinancesComponent>(raw);

                // 1. Calculate Sensitivity & Base Change
                float rent_sensitivity = 0.08f + (finances.risk_appetite * 0.04f);
                float rent_change_pct = demand_pressure * rent_sensitivity;

                // 2. Calculate Quality Adjustment (±2%)
                float quality_adj = ((quality.current_quality - 60.0f) / 100.0f) * 0.02f;

                // 3. Unconstrained Market Rent
                float raw_new_rent = rent.monthly_rent * (1.0f + rent_change_pct + quality_adj);

                // 4. Determine Bounds
                float floor_rent = finances.monthly_maintenance_cost + finances.loan_repayment;

                // 5. Apply Policy Caps & Penalties
                if (rent_control_active && raw_new_rent > rent_cap) {
                    EntityID bld_id{static_cast<uint32_t>(raw)};
                    publisher.publish(RentControlFineEvent{bld_id, raw_new_rent, rent_cap});
                    raw_new_rent = rent_cap;
                }

                // 6. Apply Floor (Prevents collapse below operating costs)
                if (raw_new_rent < floor_rent) {
                    raw_new_rent = floor_rent;
                }

                rent.monthly_rent = raw_new_rent;
            }
        }
    };
}