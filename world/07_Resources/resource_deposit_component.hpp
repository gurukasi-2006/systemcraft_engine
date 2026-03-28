#pragma once

#include "../../core/04_Types/resource_type.hpp"
#include "../../core/04_Types/fixed_point.hpp"

/**
 * @file resource_deposit_component.hpp
 * @brief ECS component representing a physical, harvestable resource deposit on a specific tile.
 */

namespace Resources {

    /**
     * @struct ResourceDepositComponent
     * @brief Attached to tile-entities to track economic materials and extraction states.
     */
    struct ResourceDepositComponent {
        /** * @brief The geological classification of the deposit (e.g., Iron, Coal, Oil, Timber, Farmland).
         */
        ResourceType type;

        /** * @brief The total remaining quantity in this deposit before it is permanently exhausted.
         */
        Fixed32 total_reserve_quantity;

        /** * @brief Multiplier for how difficult/slow this deposit is to harvest.
         * @details e.g., 1.0f is standard, 3.0f means it takes 3x longer to extract.
         */
        Fixed32 extraction_difficulty;
    };
}