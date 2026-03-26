#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>

#include "../../core/04_Types/fixed_point.hpp"
#include "../../core/04_Types/math_utils.hpp"

/**
 * @file climate_zone_assigner.hpp
 * @brief Classifies tiles into macro-climates based on latitude and elevation.
 * @details Tuned for "Systemcraft" planetary physics to ensure high-altitude mountains freeze correctly.
 */

enum class ClimateZone : uint8_t {
    Tropical,
    Temperate,
    Arid,
    Polar
};

namespace ClimateZoneAssigner {

    /**
     * @brief Determines the climate zone of a tile using a latitude and altitude simulation.
     */
    constexpr ClimateZone determine_zone(
        int32_t y_coord,
        int32_t map_height,
        Fixed32 elevation,
        Fixed32 moisture)
    {
        // 1. Calculate Latitude (Distance from Equator)
        Fixed32 equator = Fixed32(static_cast<float>(map_height)) / Fixed32(2.0f);
        Fixed32 y_pos = Fixed32(static_cast<float>(y_coord));
        Fixed32 distance_from_equator = y_pos > equator ? y_pos - equator : equator - y_pos;
        Fixed32 normalized_latitude = distance_from_equator / equator;

        // 2. Base Temperature Simulation (1.0 = Equator Heat, 0.0 = Pole Cold)
        Fixed32 base_temp = Fixed32(1.0f) - normalized_latitude;

        // 3. --- THE TUNING FIX ---
        // Increased the Lapse Rate to 0.9f. This ensures that high-elevation (0.8+)
        // tiles lose enough heat to cross the Polar threshold even at the equator.
        constexpr Fixed32 ELEVATION_LAPSE_RATE(0.9f);
        Fixed32 effective_temp = base_temp - (elevation * ELEVATION_LAPSE_RATE);

        // 4. --- THRESHOLD REFINEMENT ---
        // Raised Polar threshold to 0.25f to capture high-altitude Alpine zones.
        if (effective_temp <= Fixed32(0.25f)) {
            return ClimateZone::Polar;
        }

        if (moisture <= Fixed32(0.3f)) {
            return ClimateZone::Arid;
        }

        if (effective_temp >= Fixed32(0.7f)) {
            return ClimateZone::Tropical;
        }

        return ClimateZone::Temperate;
    }
}