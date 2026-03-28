#pragma once

#include <vector>
#include <cstdint>
#include <entt/entt.hpp>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/resource_type.hpp"
#include "deposit_registry.hpp"
#include "resource_deposit_component.hpp"

/**
 * @file resource_overlay_data_provider.hpp
 * @brief Aggregates ECS resource data into contiguous flat arrays for rapid UI heatmap rendering.
 */

namespace Resources {

    class ResourceOverlayDataProvider {
    public:
        /**
         * @brief Generates a raw float array representing the richness of a specific resource across the map.
         * @param width The map width.
         * @param height The map height.
         * @param deposit_index The spatial lookup index for O(1) tile access.
         * @param reg The master EnTT registry containing the actual deposit sizes.
         * @param target_type The specific resource to generate a heatmap for (e.g., Iron).
         * @return A flat 1D vector of floats, mapping directly to the 2D grid coordinates.
         */
        static std::vector<float> generate_heatmap(
            uint32_t width,
            uint32_t height,
            const DepositRegistry& deposit_index,
            const entt::registry& reg,
            ResourceType target_type)
        {
            // Pre-allocate the entire grid, defaulting to 0.0f (Empty)
            std::vector<float> heatmap(static_cast<size_t>(width) * height, 0.0f);

            for (uint32_t y = 0; y < height; ++y) {
                for (uint32_t x = 0; x < width; ++x) {
                    TileCoord coord{static_cast<int32_t>(x), static_cast<int32_t>(y)};

                    // Grab all deposits sitting on this specific tile
                    const auto& deposits = deposit_index.get_deposits(coord);

                    if (deposits.empty()) continue;

                    float tile_total = 0.0f;

                    // Aggregate the reserves (a single tile might have multiple veins of the same resource)
                    for (const EntityID& eid : deposits) {
                        entt::entity raw_id = static_cast<entt::entity>(eid.raw_id);

                        // Safety check to ensure the entity hasn't been destroyed mid-tick
                        if (reg.valid(raw_id) && reg.all_of<ResourceDepositComponent>(raw_id)) {
                            const auto& comp = reg.get<ResourceDepositComponent>(raw_id);

                            // Filter for the specific UI overlay requested
                            if (comp.type == target_type) {
                                // Translate from deterministic physics math to fast rendering float
                                tile_total += comp.total_reserve_quantity.toFloat();
                            }
                        }
                    }

                    // Assign to the flat array
                    size_t idx = static_cast<size_t>(y) * width + static_cast<size_t>(x);
                    heatmap[idx] = tile_total;
                }
            }

            return heatmap;
        }
    };
}