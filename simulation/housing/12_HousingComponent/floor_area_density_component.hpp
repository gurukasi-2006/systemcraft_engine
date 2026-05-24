#pragma once

#include <cstdint>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/04_Types/entity_id.hpp"

#include "zoning_compliance_component.hpp" // To enforce FAR limits
#include "building_identity_component.hpp"

/**
 * @file floor_area_density_component.hpp
 * @brief Subsystem 153: Tracks physical volume, density limits, and triggers zoning violations.
 */

namespace Housing {

    /**
     * @struct FloorAreaDensityComponent
     * @brief Manages the physical footprint and verticality of a building.
     */
    struct FloorAreaDensityComponent {
        float gross_floor_area_m2{100.0f};
        uint32_t num_floors{1};
        float plot_size_m2{200.0f};
        float floor_area_ratio{0.5f};

        /**
         * @brief Recalculates the Floor Area Ratio (FAR).
         */
        void recalculate_far() {
            if (plot_size_m2 > 0.0f) {
                floor_area_ratio = gross_floor_area_m2 / plot_size_m2;
            } else {
                floor_area_ratio = 0.0f;
            }
        }

        /**
         * @brief Checks if the building qualifies as high density.
         */
        bool is_high_density() const {
            return floor_area_ratio > 3.0f;
        }

        /**
         * @brief Checks if the building has the required volume to be classified as a Tower.
         */
        bool is_tower_eligible() const {
            return floor_area_ratio > 6.0f;
        }

        /**
         * @brief Returns the legal maximum FAR for a given zone type.
         */
        float get_max_permitted_far(ZoneType zone) const {
            switch (zone) {
                case ZoneType::ResLow:     return 0.8f;
                case ZoneType::ResHigh:    return 4.0f;
                case ZoneType::Mixed:      return 5.0f;
                case ZoneType::Commercial: return 8.0f;
                case ZoneType::Industrial: return 2.0f;
                case ZoneType::Protected:  return 0.0f; // No development allowed
                default:                   return 1.0f;
            }
        }

        /**
         * @brief Evaluates density and immediately flags the building if it violates local zoning laws.
         * @param world The ECS world to query for the Zoning Compliance component.
         * @param self_id The ID of this building entity.
         */
        void process_density(ECSWorld& world, EntityID self_id) {
            recalculate_far();

            entt::entity raw = static_cast<entt::entity>(self_id.raw_id);
            if (!world.registry.valid(raw)) return;

            if (world.registry.all_of<ZoningComplianceComponent>(raw)) {
                auto& zoning = world.registry.get<ZoningComplianceComponent>(raw);
                float max_far = get_max_permitted_far(zoning.zone_type);

                // If FAR exceeds the zone's legal limit, instantly flag for non-compliance
                if (floor_area_ratio > max_far) {
                    zoning.is_compliant = false;
                }
            }
        }
    };
}