#pragma once

#include <cstdint>

#include "../../core/04_Types/terrain_type.hpp"
#include "../../core/04_Types/fixed_point.hpp"
#include "../../core/04_Types/math_utils.hpp"

/**
 * @file construction_cost_calculator.hpp
 * @brief Evaluates environmental penalties to generate deterministic construction cost multipliers.
 */

/**
 * @enum StructureClass
 * @brief Broad classifications of buildings to determine how severely they are affected by terrain.
 */
enum class StructureClass : uint8_t {
    Light,          ///< Housing, light commercial. Standard footprint.
    Heavy,          ///< Factories, power plants. Highly sensitive to gradients and swampy ground.
    Infrastructure, ///< Roads, rail. Extremely sensitive to gradients.
    Maritime        ///< Ports, offshore rigs. Requires water, immune to swamp penalties.
};

/**
 * @namespace ConstructionCostCalculator
 * @brief Pure functions for calculating economic modifiers based on physical terrain data.
 */
namespace ConstructionCostCalculator {

    /**
     * @brief Calculates the total construction cost multiplier for a specific tile.
     * @param structure_type The classification of the building being constructed.
     * @param terrain The base geological biome (dictates rock hardness/water presence).
     * @param gradient The absolute steepness of the tile (elevation delta).
     * @param moisture The water saturation level (dictates foundation stability).
     * @return A Fixed32 multiplier (e.g., 1.0 is base cost, 2.5 is 250% cost).
     */
    constexpr Fixed32 calculate_modifier(
        StructureClass structure_type,
        TerrainType terrain,
        Fixed32 gradient,
        Fixed32 moisture)
    {
        // Baseline cost is always 1.0x (100%)
        Fixed32 multiplier(1.0f);

        // 1. Geological / Hardness Penalty
        switch (terrain) {
            case TerrainType::Mountain:
                multiplier = multiplier + Fixed32(1.5f); // Hard rock blasting required
                break;
            case TerrainType::Hills:
                multiplier = multiplier + Fixed32(0.5f); // Moderate excavation
                break;
            case TerrainType::Ocean:
            case TerrainType::Coast:
                if (structure_type != StructureClass::Maritime) {
                    // Massive penalty for land-based structures (simulates expensive land reclamation)
                    multiplier = multiplier + Fixed32(5.0f);
                } else {
                    // Maritime structures belong here, minimal base penalty
                    multiplier = multiplier + Fixed32(0.5f);
                }
                break;
            case TerrainType::Desert:
                multiplier = multiplier + Fixed32(0.3f); // Shifting sand foundations
                break;
            default:
                break;
        }

        // 2. Steepness (Gradient) Penalty
        // The steeper the slope, the higher the retaining wall / leveling costs.
        Fixed32 base_gradient_penalty = gradient * Fixed32(2.0f);

        if (structure_type == StructureClass::Heavy) {
            // Heavy industry requires massive flat concrete pads
            multiplier = multiplier + (base_gradient_penalty * Fixed32(2.0f));
        } else if (structure_type == StructureClass::Infrastructure) {
            // Roads and rails must strictly conform to gradients
            multiplier = multiplier + (base_gradient_penalty * Fixed32(3.0f));
        } else {
            multiplier = multiplier + base_gradient_penalty;
        }

        // 3. Hydrological (Swamp/Mud) Penalty
        // High moisture on land requires deep-driven friction piles to prevent sinking.
        if (moisture > Fixed32(0.7f) && terrain != TerrainType::Ocean && terrain != TerrainType::Coast) {
            Fixed32 excess_water = moisture - Fixed32(0.7f);

            if (structure_type == StructureClass::Heavy) {
                multiplier = multiplier + (excess_water * Fixed32(4.0f)); // Factories sink in mud
            } else {
                multiplier = multiplier + (excess_water * Fixed32(2.0f));
            }
        }

        // Guarantee the multiplier never mathematically drops below a 50% discount
        if (multiplier < Fixed32(0.5f)) {
            return Fixed32(0.5f);
        }

        return multiplier;
    }
}