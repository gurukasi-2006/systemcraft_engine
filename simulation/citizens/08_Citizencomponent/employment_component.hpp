#pragma once

#include <cstdint>
#include <algorithm>

#include "../../../core/04_Types/entity_id.hpp"
#include "../../../core/04_Types/fixed_point.hpp"

/**
 * @file employment_component.hpp
 * @brief Subsystem 94: Tracks citizen employment status, wages, and calculates effective productivity based on health.
 */

namespace Population {

    /**
     * @enum EmploymentStatus
     * @brief The macroeconomic status of the citizen.
     */
    enum class EmploymentStatus : uint8_t {
        Employed = 0,
        Unemployed = 1,
        Student = 2,
        Retired = 3
    };

    /**
     * @enum JobType
     * @brief Broad classifications of labor (expandable based on your economic engine needs).
     */
    enum class JobType : uint8_t {
        None = 0,
        ManualLabor,
        Service,
        Professional,
        Management
    };

    /**
     * @struct EmploymentComponent
     * @brief ECS Component holding job details and driving the calculation of daily/monthly labor output.
     */
    struct EmploymentComponent {
        EntityID employer{0};              ///< The ECS ID of the factory, shop, or government building
        JobType job_type{JobType::None};
        Fixed32 wage{0.0f};                ///< Hourly wage
        uint8_t work_hours_per_day{0};     ///< Expected standard shift (e.g., 8 hours)
        float commute_distance{0.0f};      ///< Distance in tiles to the workplace
        EmploymentStatus status{EmploymentStatus::Unemployed};

        /**
         * @brief Calculates the actual productive hours logged by the citizen today.
         * @details Reduced by poor health (working sick) or nullified entirely if absent.
         * @param physical_health The citizen's current health (from HealthComponent, 0.0 to 100.0).
         * @param present_flag True if the citizen actually arrived at work, False if skipping/sick day.
         * @return The effective fractional hours worked.
         */
        inline float calculate_effective_hours(float physical_health, bool present_flag) const {
            if (!present_flag) {
                return 0.0f; // Absent workers produce nothing
            }

            // Ensure health stays within logical bounds for the multiplier
            float health_factor = std::clamp(physical_health, 0.0f, 100.0f) / 100.0f;

            return static_cast<float>(work_hours_per_day) * health_factor;
        }

        /**
         * @brief Calculates the total monthly wage to be deposited into the citizen's savings.
         * @param effective_hours_per_day The averaged or accumulated hours worked per day this month.
         * @param working_days_in_month The number of days the citizen was scheduled to work.
         * @return Total gross income for the period.
         */
        inline Fixed32 calculate_monthly_credit(float effective_hours_per_day, int32_t working_days_in_month) const {
            // monthly_credit = wage * effective_hours * working_days_in_month
            Fixed32 hours_fixed = Fixed32(effective_hours_per_day);
            Fixed32 days_fixed = Fixed32(working_days_in_month);

            return wage * hours_fixed * days_fixed;
        }
    };
}