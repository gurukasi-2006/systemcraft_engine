#pragma once

#include <cstdint>
#include <algorithm>

#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/entity_id.hpp"
#include "../../../core/04_Types/time_constants.hpp"

/**
 * @file rent_valuation_component.hpp
 * @brief Subsystem 144: Tracks building valuation, yields, and fires housing bubble warnings.
 */

namespace Housing {

    /**
     * @enum OwnershipType
     * @brief Determines the profit-viability thresholds for the building.
     */
    enum class OwnershipType : uint8_t {
        Private = 0,    // Seeks > 5% Net Yield
        Social,         // Seeks > 0% Net Yield (Break-even)
        Government      // Accepts negative yields (Subsidized)
    };

    /**
     * @struct BubbleWarningEvent
     * @brief Fired when a building's valuation becomes dangerously decoupled from its rent reality.
     */
    struct BubbleWarningEvent {
        EntityID building_id;
        float price_to_rent_ratio;
        float current_valuation;
    };

    /**
     * @struct RentValuationComponent
     * @brief The core financial ledger for a residential structure.
     */
    struct RentValuationComponent {
        float monthly_rent{1000.0f};
        float market_valuation{250000.0f};
        OwnershipType ownership_type{OwnershipType::Private};

        uint64_t last_adjustment_tick{0};

        // Static Building Specs used for math
        float floor_area_m2{100.0f};
        float cost_per_m2{1500.0f};
        float annual_maintenance_cost{2000.0f};

        /**
         * @brief Recalculates the fundamental market valuation of the property.
         */
        void update_valuation(float structural_quality, float land_value_tile, float density_bonus, float location_score, float neighbourhood_premium) {

            float quality_multiplier = std::max(0.01f, structural_quality / 100.0f);

            float base_construction_value = floor_area_m2 * cost_per_m2 * quality_multiplier;
            float land_value = land_value_tile * floor_area_m2 * density_bonus;
            float localized_premium = location_score * neighbourhood_premium;

            market_valuation = base_construction_value + land_value + localized_premium;
        }

        /**
         * @brief Returns the annual rent collected.
         */
        float get_annual_rent() const {
            return monthly_rent * 12.0f;
        }

        /**
         * @brief Gross Yield = (Annual Rent / Market Valuation) * 100
         */
        float get_gross_yield() const {
            if (market_valuation <= 0.0f) return 0.0f;
            return (get_annual_rent() / market_valuation) * 100.0f;
        }

        /**
         * @brief Net Yield = Gross Yield - (Maintenance / Market Valuation) * 100
         */
        float get_net_yield() const {
            if (market_valuation <= 0.0f) return 0.0f;
            float gross = get_gross_yield();
            float maintenance_impact = (annual_maintenance_cost / market_valuation) * 100.0f;
            return gross - maintenance_impact;
        }

        /**
         * @brief Evaluates if the building is financially viable for its owner.
         */
        bool is_financially_viable() const {
            float net = get_net_yield();
            switch (ownership_type) {
                case OwnershipType::Private:    return net > 5.0f;
                case OwnershipType::Social:     return net > 0.0f;
                case OwnershipType::Government: return true; // Always viable
            }
            return false;
        }

        /**
         * @brief Checks the Price-to-Earnings ratio and fires an event if a bubble is forming.
         * @details P/E > 25x is considered a dangerous bubble.
         */
        void check_bubble_risk(EventPublisher& publisher, EntityID self_id) const {
            float annual_rent = get_annual_rent();
            if (annual_rent <= 0.0f) return;

            float pe_ratio = market_valuation / annual_rent;

            if (pe_ratio > 25.0f) {
                publisher.publish(BubbleWarningEvent{
                    self_id,
                    pe_ratio,
                    market_valuation
                });
            }
        }
    };
}