#pragma once

#include <cstdint>
#include <array>
#include <cmath>

// Pull in the Mincer equation coefficient from your economic constants
#include "../../../core/04_Types/economic_constants.hpp"

/**
 * @file education_component.hpp
 * @brief Subsystem 93: Tracks citizen education, qualifications, and category-specific job skills.
 */

namespace Population {

    /**
     * @enum QualificationLevel
     * @brief The highest formal academic degree achieved by the citizen.
     */
    enum class QualificationLevel : uint8_t {
        None = 0,
        Primary = 1,
        Secondary = 2,
        HigherSecondary = 3,
        UG = 4,   ///< Undergraduate (Bachelor's)
        PG = 5,   ///< Postgraduate (Master's)
        PhD = 6   ///< Doctorate
    };

    /**
     * @enum JobCategory
     * @brief Broad economic sectors where citizens can accumulate specialized skill.
     */
    enum class JobCategory : uint8_t {
        Agriculture = 0,
        Extraction,     ///< Mining, Drilling, Logging
        Manufacturing,  ///< Factories, Assembly
        Construction,
        Retail,         ///< Commercial services
        Healthcare,
        Education,
        Administration, ///< Bureaucracy, Office work
        Research,       ///< Tech lab jobs
        COUNT           ///< Automatically tracks the number of categories
    };

    /**
     * @struct EducationComponent
     * @brief ECS Component holding the citizen's human capital and driving their wage expectations.
     */
    struct EducationComponent {
        float years_of_schooling{0.0f};
        QualificationLevel qualification{QualificationLevel::None};
        bool enrolled{false};

        // A fixed array holding the specific skill level (0.0 to N) for every possible job sector.
        // Initializes all skills to 0.0f.
        std::array<float, static_cast<size_t>(JobCategory::COUNT)> skill_levels{};

        /**
         * @brief Calculates the citizen's expected wage using the Mincer earnings function.
         * @details wage = base_wage * exp(SKILL_PREMIUM_COEFFICIENT * skill_level)
         * @param base_wage The current national or regional base wage for unskilled labor.
         * @param target_job The specific sector the citizen is working in (or applying for).
         * @return The final adjusted wage.
         */
        inline float calculate_expected_wage(float base_wage, JobCategory target_job) const {
            float specific_skill = skill_levels[static_cast<size_t>(target_job)];

            // The Mincer Premium: e^(0.08 * skill)
            float premium_multiplier = std::exp(Labour::SKILL_PREMIUM_COEFFICIENT * specific_skill);

            return base_wage * premium_multiplier;
        }

        /**
         * @brief Adds on-the-job experience to a specific sector.
         * @param current_job The sector the citizen is working in.
         * @param experience_gained The amount of skill points to add this tick/month.
         */
        inline void add_job_experience(JobCategory current_job, float experience_gained) {
            skill_levels[static_cast<size_t>(current_job)] += experience_gained;
        }
    };
}