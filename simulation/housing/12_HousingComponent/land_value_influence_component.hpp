#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>

#include "../../../core/04_Types/tile_coord.hpp"
#include "building_identity_component.hpp" // For BuildingType

/**
 * @file land_value_influence_component.hpp
 * @brief Subsystem 149: Calculates continuous spatial fields for land value auras.
 */

namespace Housing {

    /**
     * @struct LandValueInfluenceComponent
     * @brief Emits a localized economic aura that decays exponentially over distance.
     */
    struct LandValueInfluenceComponent {
        uint32_t influence_radius{5};

        /**
         * @brief Returns the raw economic weight multiplier of the building classification.
         */
        float get_type_strength(BuildingType type) const {
            switch (type) {
                case BuildingType::Shack:      return 1.0f;
                case BuildingType::House:      return 2.0f;
                case BuildingType::LowRiseApt: return 4.0f;
                case BuildingType::Tower:      return 8.0f;
                default:                       return 1.0f;
            }
        }

        /**
         * @brief Calculates the epicenter emission strength of the building.
         * @details Formula: base_emit = (quality/100.0 - 0.5) * type_strength
         */
        float get_base_emit(float structural_quality, BuildingType type) const {
            float quality_factor = (structural_quality / 100.0f) - 0.5f;
            return quality_factor * get_type_strength(type);
        }

        /**
         * @brief Standard Chebyshev distance calculation (Diagonal moves cost 1).
         */
        uint32_t get_chebyshev_distance(TileCoord source, TileCoord target) const {
            int32_t dx = std::abs(source.x - target.x);
            int32_t dy = std::abs(source.y - target.y);
            return static_cast<uint32_t>(std::max(dx, dy));
        }

        /**
         * @brief Calculates the exact economic influence this building exerts on a specific tile.
         * @details Formula: influence(d) = base_emit * exp(-0.3 * d)
         */
        float get_influence_at_target(float structural_quality, BuildingType type, TileCoord source, TileCoord target) const {
            uint32_t distance = get_chebyshev_distance(source, target);

            // Beyond the theoretical radius, influence flatlines to 0
            if (distance > influence_radius) {
                return 0.0f;
            }

            float base_emit = get_base_emit(structural_quality, type);

            // Exponential decay continuous field
            return base_emit * std::exp(-0.3f * static_cast<float>(distance));
        }
    };
}