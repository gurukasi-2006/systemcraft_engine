#pragma once

#include <unordered_map>
#include <cstdint>

#include "../../core/04_Types/resource_type.hpp"
#include "../../core/04_Types/fixed_point.hpp"
#include "../../core/03_Event_Bus/immediate_dispatcher.hpp"
#include "import_export_resource_ledger.hpp"

/**
 * @file resource_price_signal.hpp
 * @brief Calculates a scarcity multiplier based on macroeconomic supply/demand and broadcasts it to the Market.
 */

namespace Resources {

    /**
     * @struct PriceSignalEvent
     * @brief Blasted to the Market System every tick to dynamically adjust trading costs.
     */
    struct PriceSignalEvent {
        ResourceType type;
        Fixed32 scarcity_multiplier; ///< 1.0 = Balanced, > 1.0 = Shortage/Expensive, < 1.0 = Surplus/Cheap
    };

    /**
     * @class ResourcePriceSignal
     * @brief Computes resource scarcity and emits price hints based on ledger statistics.
     */
    class ResourcePriceSignal {
    private:
        ImmediateDispatcher& dispatcher_;

        // Track the running consumption rate (units burned/used per tick by factories/cities)
        // Note: In a full engine, this would likely be fed by a separate Consumption Ledger,
        // but we track it locally here for the signal calculation.
        std::unordered_map<ResourceType, Fixed32> consumption_rates_;

    public:
        /**
         * @brief Constructs the signal calculator with a link to the real-time event router.
         */
        explicit ResourcePriceSignal(ImmediateDispatcher& dispatcher)
            : dispatcher_(dispatcher) {}

        /**
         * @brief Records how much of a resource was burned/used by the nation this tick.
         */
        void record_consumption(ResourceType type, Fixed32 amount) {
            consumption_rates_[type] = consumption_rates_[type] + amount;
        }

        /**
         * @brief Calculates the scarcity index and fires the pricing event.
         * @param ledger The master financial ledger containing extraction/trade totals.
         * @param target_type The specific resource to evaluate.
         */
        void emit_signal(const ImportExportResourceLedger& ledger, ResourceType target_type) {

            ResourceAnnualStats stats = ledger.get_stats(target_type);
            Fixed32 supply = stats.get_net_domestic_supply();

            Fixed32 demand = Fixed32(0.0f);
            if (consumption_rates_.find(target_type) != consumption_rates_.end()) {
                demand = consumption_rates_[target_type];
            }

            // --- The Scarcity Algorithm ---
            Fixed32 scarcity_multiplier(1.0f); // Default balanced price

            if (demand > Fixed32(0.0f)) {
                if (supply <= Fixed32(0.0f)) {
                    // Critical Shortage: High demand, zero domestic supply. Price spikes massively.
                    scarcity_multiplier = Fixed32(3.0f);
                } else {
                    // Standard Market Equation: Demand / Supply
                    // E.g., Demand 200 / Supply 100 = 2.0x Scarcity (Price doubles)
                    // E.g., Demand 50 / Supply 200 = 0.25x Scarcity (Price crashes due to oversupply)
                    scarcity_multiplier = demand / supply;

                    // Clamp to prevent infinite hyper-inflation or absolute zero crashes
                    if (scarcity_multiplier > Fixed32(5.0f)) scarcity_multiplier = Fixed32(5.0f);
                    if (scarcity_multiplier < Fixed32(0.1f)) scarcity_multiplier = Fixed32(0.1f);
                }
            } else if (supply > Fixed32(0.0f) && demand == Fixed32(0.0f)) {
                 // Absolute Surplus: No one is buying it, but we have tons. Price floors out.
                 scarcity_multiplier = Fixed32(0.1f);
            }

            // Dispatch the economic hint to the wider engine
            dispatcher_.dispatch(PriceSignalEvent{target_type, scarcity_multiplier});

            // INDUSTRIAL FAILSAFE: Auto-reset the consumption tracker for this resource
            // to prevent creeping hyperinflation if the global tick reset is missed.
            consumption_rates_.erase(target_type);
        }}

        /**
         * @brief Resets consumption trackers (usually called at the start of a new calculation cycle).
         */
        void reset_tick() {
            consumption_rates_.clear();
        }
    };
}
