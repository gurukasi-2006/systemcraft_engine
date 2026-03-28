#pragma once

#include <unordered_map>
#include <cstdint>

#include "../../core/04_Types/resource_type.hpp"
#include "../../core/04_Types/fixed_point.hpp"
#include "../../core/03_Event_Bus/immediate_dispatcher.hpp"

/**
 * @file strategic_reserve_manager.hpp
 * @brief Enforces government stockpile thresholds and triggers emergency economic actions.
 */

namespace Resources {

    /**
     * @struct EmergencyImportEvent
     * @brief Blasted to the Market System to forcefully purchase missing strategic resources.
     */
    struct EmergencyImportEvent {
        ResourceType type;
        Fixed32 requested_amount;
    };

    /**
     * @struct ExtractionBonusEvent
     * @brief Blasted to the Extraction System to temporarily boost domestic mining yields.
     */
    struct ExtractionBonusEvent {
        ResourceType type;
        Fixed32 multiplier;
    };

    /**
     * @class StrategicReserveManager
     * @brief Holds national security thresholds and evaluates them against current stockpiles.
     */
    class StrategicReserveManager {
    private:
        ImmediateDispatcher& dispatcher_;
        std::unordered_map<ResourceType, Fixed32> minimum_thresholds_;

    public:
        /**
         * @brief Constructs the manager with a link to the real-time event router.
         */
        explicit StrategicReserveManager(ImmediateDispatcher& dispatcher)
            : dispatcher_(dispatcher) {}

        /**
         * @brief Sets the national security minimum for a specific resource.
         */
        void set_minimum_threshold(ResourceType type, Fixed32 threshold) {
            minimum_thresholds_[type] = threshold;
        }

        /**
         * @brief Retrieves the current policy threshold for a resource.
         */
        Fixed32 get_minimum_threshold(ResourceType type) const {
            auto it = minimum_thresholds_.find(type);
            if (it != minimum_thresholds_.end()) {
                return it->second;
            }
            return Fixed32(0.0f); // Default: No strategic reserve mandated
        }

        /**
         * @brief Evaluates the current physical stockpile against the mandated threshold.
         * @param type The resource to check.
         * @param current_stockpile The actual amount currently sitting in national warehouses.
         */
        void evaluate_reserve(ResourceType type, Fixed32 current_stockpile) {
            Fixed32 threshold = get_minimum_threshold(type);

            // Only trigger if a threshold is actually set AND we are below it
            if (threshold > Fixed32(0.0f) && current_stockpile < threshold) {
                Fixed32 deficit = threshold - current_stockpile;

                // 1. Order the market to buy the exact missing amount
                dispatcher_.dispatch(EmergencyImportEvent{type, deficit});

                // 2. Grant a 50% yield bonus to domestic extractors to fix the shortage locally
                dispatcher_.dispatch(ExtractionBonusEvent{type, Fixed32(1.5f)});
            }
        }
    };
}