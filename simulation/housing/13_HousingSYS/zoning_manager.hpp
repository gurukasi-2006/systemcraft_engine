#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "../../../core/04_Types/tile_coord.hpp"
#include "../../../core/04_Types/time_constants.hpp"

// Reuse the enums we defined in Phase 4 Components
#include "../12_HousingComponent/zoning_compliance_component.hpp"
#include "../12_HousingComponent/building_identity_component.hpp"

/**
 * @file zoning_manager.hpp
 * @brief Subsystem 154: Governs the 2D zone map, construction eligibility, and temporal mixed-use logic.
 */

namespace Housing {

    /**
     * @enum DevelopmentCategory
     * @brief Maps physical building types into zoning categories.
     */
    enum class DevelopmentCategory : uint8_t {
        ShackHouse = 0,
        LowRiseApt,
        Tower,
        CommercialBld,
        FactoryBld
    };

    /**
     * @class ZoningManager
     * @brief Stores the painted zones and enforces the city's urban planning code.
     */
    class ZoningManager {
    private:
        // We pack the TileCoord (x, y) into a 64-bit int for fast, hash-safe 2D grid lookups
        std::unordered_map<uint64_t, ZoneType> zone_map_;
        float base_rezoning_cost_{500.0f}; // Cost per tile to paint a new zone

        uint64_t get_tile_key(TileCoord coord) const {
            return (static_cast<uint64_t>(coord.x) << 32) | static_cast<uint64_t>(coord.y);
        }

    public:
        /**
         * @brief Paints a zone onto the map.
         */
        void set_zone(TileCoord coord, ZoneType type) {
            zone_map_[get_tile_key(coord)] = type;
        }

        /**
         * @brief Retrieves the zone for a given tile. Defaults to Protected if unpainted.
         */
        ZoneType get_zone(TileCoord coord) const {
            auto it = zone_map_.find(get_tile_key(coord));
            if (it != zone_map_.end()) {
                return it->second;
            }
            return ZoneType::Protected; // Unzoned land cannot be built on
        }

        /**
         * @brief Checks the construction compatibility matrix.
         */
        bool is_eligible(TileCoord coord, DevelopmentCategory category) const {
            ZoneType zone = get_zone(coord);

            switch (zone) {
                case ZoneType::ResLow:
                    return (category == DevelopmentCategory::ShackHouse || category == DevelopmentCategory::LowRiseApt);

                case ZoneType::ResHigh:
                    return (category == DevelopmentCategory::LowRiseApt || category == DevelopmentCategory::Tower);

                case ZoneType::Commercial:
                    return (category == DevelopmentCategory::CommercialBld);

                case ZoneType::Industrial:
                    return (category == DevelopmentCategory::FactoryBld);

                case ZoneType::Mixed:
                    // Mixed use allows all housing and commercial, but NO factories!
                    return (category != DevelopmentCategory::FactoryBld);

                case ZoneType::Protected:
                default:
                    return false;
            }
        }

        /**
         * @brief Calculates the cost of transitioning painted zones.
         * @details Formula: affected_tiles * base_rezoning_cost * zone_disruption_factor
         */
        float calculate_rezoning_cost(ZoneType old_zone, ZoneType new_zone, uint32_t affected_tiles) const {
            if (old_zone == new_zone) return 0.0f;

            float disruption_factor = 1.0f;

            if (old_zone == ZoneType::ResLow && new_zone == ZoneType::ResHigh) {
                disruption_factor = 1.5f; // Gentrification pushback
            } else if (old_zone == ZoneType::Protected) {
                disruption_factor = 5.0f; // Massive legal fees to build on a nature reserve
            } else if (new_zone == ZoneType::Protected) {
                disruption_factor = 3.0f; // Eminent domain costs to seize and protect land
            }

            return static_cast<float>(affected_tiles) * base_rezoning_cost_ * disruption_factor;
        }

        /**
         * @brief Temporal Zoning Rule: Evaluates if ground-floor commercial in a mixed zone is active right now.
         * @param coord The tile to check.
         * @param current_tick Absolute simulation time.
         * @return True if commercial businesses are open, affecting noise and transport demand.
         */
        bool is_mixed_commercial_active(TileCoord coord, uint64_t current_tick) const {
            if (get_zone(coord) != ZoneType::Mixed) return false;

            // Extract the hour of the day (0 to 23)
            uint64_t ticks_per_hour = TimeConstants::TICKS_PER_DAY / 24;
            if (ticks_per_hour == 0) ticks_per_hour = 1; // Failsafe

            uint64_t tick_of_day = current_tick % TimeConstants::TICKS_PER_DAY;
            uint32_t hour = static_cast<uint32_t>(tick_of_day / ticks_per_hour);

            // Commercial operates 08:00 (8 AM) to 20:00 (8 PM)
            return (hour >= 8 && hour < 20);
        }
    };
}