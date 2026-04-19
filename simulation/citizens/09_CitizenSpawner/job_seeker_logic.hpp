#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <entt/entt.hpp>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/04_Types/tile_coord.hpp"

#include "../08_Citizencomponent/employment_component.hpp"
#include "../08_Citizencomponent/education_component.hpp"
#include "../08_Citizencomponent/social_network_component.hpp"

// Pull in PositionComponent defined during Spawner testing
#include "../09_CitizenSpawner/citizen_spawner.hpp"

/**
 * @file job_seeker_logic.hpp
 * @brief Subsystem 106: Unemployed citizens evaluate and accept job postings.
 */

struct JobPosting {
    EntityID employer_id;
    TileCoord location;
    Population::JobCategory category;
    float required_skill;
    float offered_wage;
};

class JobSeekerLogic {
public:
    /**
     * @brief Evaluates available postings for all unemployed citizens and places them in optimal roles.
     * @param world The ECS world containing citizens.
     * @param job_postings The active master list of job vacancies.
     * @param regional_median_wage The macroeconomic baseline for scoring offer attractiveness.
     * @param poverty_line The threshold used to calculate absolute minimum acceptable living wages.
     */
    void update(ECSWorld& world, std::vector<JobPosting>& job_postings, float regional_median_wage, float poverty_line) {

        // We require all 4 components to run the complex scoring matrix
        auto view = world.registry.view<
            Population::EmploymentComponent,
            Population::EducationComponent,
            PositionComponent,
            Population::SocialNetworkComponent
        >();

        for (auto raw_id : view) {
            auto& emp_comp = view.get<Population::EmploymentComponent>(raw_id);

            if (emp_comp.status != Population::EmploymentStatus::Unemployed) {
                continue; // Only active job seekers
            }

            auto& edu_comp = view.get<Population::EducationComponent>(raw_id);
            auto& pos_comp = view.get<PositionComponent>(raw_id);
            auto& soc_comp = view.get<Population::SocialNetworkComponent>(raw_id);

            // --- 1. Minimum Acceptable Wage Threshold ---
            // Simulates frictional unemployment: Citizens will reject offers that pay too far
            // below their previous standard of living, or below basic survival limits.
            float prev_wage = emp_comp.wage.toFloat();
            float min_acceptable = std::max(poverty_line * 0.9f, prev_wage * 0.7f);

            int best_job_index = -1;
            float best_offer_score = -9999.0f;

            // --- 2. Evaluate All Job Postings ---
            for (size_t i = 0; i < job_postings.size(); ++i) {
                const auto& job = job_postings[i];

                // Hard Filter: Citizens instantly reject insulting wages.
                if (job.offered_wage < min_acceptable) {
                    continue;
                }

                // Wage Score
                float wage_score = job.offered_wage / regional_median_wage;

                // Skill Match
                float citizen_skill = edu_comp.skill_levels[static_cast<size_t>(job.category)];
                float skill_match = 1.0f - (std::abs(citizen_skill - job.required_skill) / 10.0f);

                // Commute Penalty (Normalized: 20 tiles = max penalty)
                float dx = static_cast<float>(pos_comp.coord.x - job.location.x);
                float dy = static_cast<float>(pos_comp.coord.y - job.location.y);
                float commute_tiles = std::sqrt((dx * dx) + (dy * dy));
                float commute_pen = std::clamp(commute_tiles / 20.0f, 0.0f, 1.0f);

                // Network Bonus (Spatial Clustering)
                // If a friend works at this exact facility, they can provide a referral!
                float network_bonus = 0.0f;
                for (const auto& link : soc_comp.acquaintance_list) {
                    if (world.registry.valid(link.entity)) {
                        // Check if the friend has a job component
                        if (world.registry.any_of<Population::EmploymentComponent>(link.entity)) {
                            auto& friend_emp = world.registry.get<Population::EmploymentComponent>(link.entity);
                            if (friend_emp.status == Population::EmploymentStatus::Employed &&
                                friend_emp.employer.raw_id == job.employer_id.raw_id) {
                                network_bonus = 0.20f;
                                break; // Only need one referral
                            }
                        }
                    }
                }

                // Composite Matrix Score
                float offer_score = (0.50f * wage_score) + (0.30f * skill_match) - (0.20f * commute_pen) + network_bonus;

                if (offer_score > best_offer_score) {
                    best_offer_score = offer_score;
                    best_job_index = static_cast<int>(i);
                }
            }

            // --- 3. Acceptance & State Transition ---
            if (best_job_index != -1) {
                const auto& accepted_job = job_postings[best_job_index];

                emp_comp.employer = accepted_job.employer_id;
                emp_comp.status = Population::EmploymentStatus::Employed;
                emp_comp.wage = Fixed32(accepted_job.offered_wage);
                emp_comp.work_hours_per_day = 8; // Standard 8-hour baseline

                // Generic mapping for JobType; complex engines would carry this inside JobPosting
                emp_comp.job_type = Population::JobType::Professional;

                float dx = static_cast<float>(pos_comp.coord.x - accepted_job.location.x);
                float dy = static_cast<float>(pos_comp.coord.y - accepted_job.location.y);
                emp_comp.commute_distance = std::sqrt((dx * dx) + (dy * dy));

                // Remove the vacancy to prevent 10,000 citizens accepting 1 open role
                job_postings.erase(job_postings.begin() + best_job_index);
            }
        }
    }
};