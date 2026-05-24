#pragma once

#include <cstdint>
#include <algorithm>

#include "building_identity_component.hpp" // For BuildingType

/**
 * @file construction_progress_component.hpp
 * @brief Subsystem 147: Tracks build phases, percentage complete, and material gating.
 */

namespace Housing {

    /**
     * @enum ConstructionPhase
     * @brief The distinct physical stages of building erection.
     */
    enum class ConstructionPhase : uint8_t {
        Planned = 0,    // 0%  - Plot reserved
        Foundation,     // 10% - Cement poured
        Framing,        // 40% - Structural skeleton up
        Finishing,      // 80% - Interior and facade
        Complete        // 100% - Ready for occupants
    };

    /**
     * @struct ConstructionProgressComponent
     * @brief Manages labor input and physical resource requirements for a building site.
     */
    struct ConstructionProgressComponent {
        ConstructionPhase phase{ConstructionPhase::Planned};
        float progress_pct{0.0f};
        uint16_t workers_assigned{0};

        // --- Materials Delivered ---
        float cement_delivered{0.0f};
        float steel_delivered{0.0f};
        float timber_delivered{0.0f};
        float consumer_goods_delivered{0.0f};

        // --- Materials Required ---
        float cement_required{0.0f};
        float steel_required{0.0f};
        float timber_required{0.0f};
        float consumer_goods_required{0.0f};

        /**
         * @brief Calculates the total materials needed based on building specs.
         * @param type The BuildingType classification.
         * @param floor_area_m2 The footprint of the building.
         * @param num_floors Verticality multiplier.
         */
        void initialize_requirements(BuildingType type, float floor_area_m2, uint32_t num_floors) {
            float f_floors = static_cast<float>(num_floors);

            cement_required = floor_area_m2 * 0.15f * f_floors;
            steel_required  = floor_area_m2 * 0.08f * f_floors;

            // Towers use 80% less timber (concrete/glass/steel structure)
            float timber_mult = (type < BuildingType::Tower) ? 1.0f : 0.2f;
            timber_required = floor_area_m2 * 0.05f * timber_mult;

            // Finishing goods (wiring, plumbing, appliances)
            consumer_goods_required = floor_area_m2 * 0.02f * f_floors;
        }

        /**
         * @brief Determines the maximum allowed progress based on current material deliveries.
         */
        float get_progress_cap() const {
            // Cannot start foundation (0% -> 10%) without Cement
            if (cement_delivered < cement_required) return 0.0f;

            // Cannot start framing (10% -> 40%) without Steel and Timber
            if (steel_delivered < steel_required || timber_delivered < timber_required) return 10.0f;

            // Cannot finish the building (80% -> 100%) without Consumer Goods
            if (consumer_goods_delivered < consumer_goods_required) return 80.0f;

            return 100.0f; // Fully supplied
        }

        /**
         * @brief Applies labor to the site, progressing construction up to the material cap.
         * @param labor_effort The amount of work done this tick (translates to % progress).
         */
        void advance_construction(float labor_effort) {
            if (progress_pct >= 100.0f) return;

            float cap = get_progress_cap();

            // Apply labor
            progress_pct += labor_effort;

            // Cap progress if materials are missing
            if (progress_pct > cap) {
                progress_pct = cap;
            }

            // Update Phase State Machine
            if (progress_pct >= 100.0f)      phase = ConstructionPhase::Complete;
            else if (progress_pct >= 80.0f)  phase = ConstructionPhase::Finishing;
            else if (progress_pct >= 40.0f)  phase = ConstructionPhase::Framing;
            else if (progress_pct >= 10.0f)  phase = ConstructionPhase::Foundation;
            else                             phase = ConstructionPhase::Planned;
        }
    };
}