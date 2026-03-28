#pragma once

#include <entt/entt.hpp>
#include "../../core/04_Types/fixed_point.hpp"
#include "../../core/04_Types/resource_type.hpp"
#include "resource_deposit_component.hpp"

/**
 * @file renewable_resource_regenerator.hpp
 * @brief Passively replenishes organic resource deposits (Timber, Farmland) over time.
 */

namespace Resources {

    class RenewableResourceRegenerator {
    public:
        /**
         * @brief Sweeps the registry and regenerates renewable resources up to their natural cap.
         * @param reg The master EnTT registry.
         * @param regen_rate The amount of resource to add per tick.
         * @param max_capacity The physical limit of the tile (prevents infinite density).
         */
        static void regenerate_tick(entt::registry& reg, Fixed32 regen_rate, Fixed32 max_capacity) {
            // Retrieve all entities that still have a valid resource deposit
            auto view = reg.view<ResourceDepositComponent>();

            for (auto entity : view) {
                auto& deposit = view.get<ResourceDepositComponent>(entity);

                // Only apply regeneration to biologically renewable resources
                if (deposit.type == ResourceType::Timber || /* deposit.type == ResourceType::Farmland */
                    static_cast<uint8_t>(deposit.type) == 99) { // Placeholder if Farmland isn't in your enum yet

                    // If the deposit is completely destroyed (0.0), it cannot regrow naturally
                    if (deposit.total_reserve_quantity > Fixed32(0.0f) &&
                        deposit.total_reserve_quantity < max_capacity) {

                        deposit.total_reserve_quantity = deposit.total_reserve_quantity + regen_rate;

                        // Clamp to the environmental maximum to prevent infinite overflow
                        if (deposit.total_reserve_quantity > max_capacity) {
                            deposit.total_reserve_quantity = max_capacity;
                        }
                    }
                }
            }
        }
    };
}