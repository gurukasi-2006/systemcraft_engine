#pragma once

#include <cstdint>

#include "../../../core/04_Types/entity_id.hpp"
#include "../../../core/04_Types/tile_coord.hpp"

/**
 * @file building_identity_component.hpp
 * @brief Subsystem 141: Foundational immutable data for all residential structures.
 */

namespace Housing {

    /**
     * @enum BuildingType
     * @brief Defines the classification, capacity, and density of a residential building.
     */
    enum class BuildingType : uint8_t {
        Shack = 0,
        House,
        LowRiseApt,
        Tower,
        COUNT
    };

    /**
     * @struct BuildingIdentityComponent
     * @brief Immutable core data defining a building entity at construction.
     */
    struct BuildingIdentityComponent {
        EntityID building_id{0};          // Buildings share the global ECS EntityID space
        uint64_t construction_tick{0};    // The exact simulation tick the building was finished
        TileCoord address{0, 0};          // Spatial key for SpatialIndex queries
        BuildingType type{BuildingType::Shack};
    };

}