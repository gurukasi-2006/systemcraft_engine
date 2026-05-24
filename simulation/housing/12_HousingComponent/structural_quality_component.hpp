#pragma once

#include <cstdint>
#include <algorithm>

/**
 * @file structural_quality_component.hpp
 * @brief Subsystem 142: Tracks physical condition of buildings, accelerating degradation, and maintenance constraints.
 */

namespace Housing {

    /**
     * @enum BuildingConditionStatus
     * @brief Quality thresholds that trigger rent, health, and policy actions.
     */
    enum class BuildingConditionStatus : uint8_t {
        CollapseRisk = 0, // 0 - 4:   Emergency eviction, 0x rent, -0.02 health
        Derelict,         // 5 - 19:  Demolition candidate, 0.25x rent, -0.008 health
        Poor,             // 20 - 39: Work order created, 0.55x rent, -0.003 health
        Deteriorating,    // 40 - 59: Maintenance warning, 0.80x rent, -0.001 health
        Good,             // 60 - 79: 1.0x rent, 0 health penalty
        Excellent         // 80 - 100: 1.20x rent, 0 health penalty
    };

    /**
     * @struct StructuralQualityComponent
     * @brief The living health-bar of a building entity.
     */
    struct StructuralQualityComponent {
        float current_quality{100.0f};

        /**
         * @brief Degrades the building's quality for a single tick.
         * @details Degradation accelerates as quality drops: dQ/dt = -(0.002 + 0.005 * (100 - Q) / 100)
         */
        void apply_degradation() {
            // Accelerating tail: the lower the quality, the higher the penalty
            float penalty_ratio = (100.0f - current_quality) / 100.0f;
            float degradation = 0.002f + (0.005f * penalty_ratio);

            current_quality -= degradation;
            if (current_quality < 0.0f) {
                current_quality = 0.0f;
            }
        }

        /**
         * @brief Determines the current threshold status of the building.
         */
        BuildingConditionStatus get_status() const {
            if (current_quality >= 80.0f) return BuildingConditionStatus::Excellent;
            if (current_quality >= 60.0f) return BuildingConditionStatus::Good;
            if (current_quality >= 40.0f) return BuildingConditionStatus::Deteriorating;
            if (current_quality >= 20.0f) return BuildingConditionStatus::Poor;
            if (current_quality >= 5.0f)  return BuildingConditionStatus::Derelict;
            return BuildingConditionStatus::CollapseRisk;
        }

        /**
         * @brief Returns the rent multiplier based on the current status.
         */
        float get_rent_modifier() const {
            switch (get_status()) {
                case BuildingConditionStatus::Excellent:     return 1.20f;
                case BuildingConditionStatus::Good:          return 1.00f;
                case BuildingConditionStatus::Deteriorating: return 0.80f;
                case BuildingConditionStatus::Poor:          return 0.55f;
                case BuildingConditionStatus::Derelict:      return 0.25f;
                case BuildingConditionStatus::CollapseRisk:  return 0.00f;
            }
            return 1.0f;
        }

        /**
         * @brief Returns the per-tick health penalty for occupants based on current status.
         */
        float get_health_penalty() const {
            switch (get_status()) {
                case BuildingConditionStatus::Excellent:     return 0.000f;
                case BuildingConditionStatus::Good:          return 0.000f;
                case BuildingConditionStatus::Deteriorating: return 0.001f;
                case BuildingConditionStatus::Poor:          return 0.003f;
                case BuildingConditionStatus::Derelict:      return 0.008f;
                case BuildingConditionStatus::CollapseRisk:  return 0.020f;
            }
            return 0.0f;
        }

        /**
         * @brief Restores building quality based on contractor expertise.
         * @details Formula: 90 + contractor_skill * 0.5.
         * Max theoretical skill is 10 (Restores to 95). Never reaches 100 (Irreversible wear).
         */
        void apply_maintenance(float contractor_skill) {
            float restored_quality = 90.0f + (contractor_skill * 0.5f);

            // Hard clamp at 99.9 to represent permanent micro-fissures / wear-and-tear
            current_quality = std::min(restored_quality, 99.9f);
        }
    };
}