#pragma once

#include <entt/entt.hpp>
#include <unordered_map>
#include <array>
#include <algorithm>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/tile_coord.hpp"

#include "../08_Citizencomponent/age_lifecycle_component.hpp"
#include "../08_Citizencomponent/education_component.hpp"
#include "../09_CitizenSpawner/citizen_spawner.hpp" // For PositionComponent

/**
 * @file education_level_tracker.hpp
 * @brief Subsystem 132: Tracks workforce qualifications to unlock high-tech industries and calculate productivity bonuses.
 */

namespace Demographics {

    /**
     * @enum Qualification
     * @brief The highest attained education level of a citizen.
     */
    enum class Qualification : uint8_t {
        None = 0,
        Primary,
        Secondary,
        Tertiary,
        COUNT
    };

    /**
     * @struct RegionalEducationData
     * @brief Holds the demographic education breakdown for a specific region.
     */
    struct RegionalEducationData {
        uint32_t working_age_population{0};
        std::array<uint32_t, static_cast<size_t>(Qualification::COUNT)> edu_histogram{};

        float tertiary_ratio{0.0f};
        float regional_productivity_bonus{1.0f};
    };

    /**
     * @struct EducationDemographicsEvent
     * @brief The monthly statistical packet sent to the Economy and UI systems.
     */
    struct EducationDemographicsEvent {
        std::unordered_map<uint32_t, RegionalEducationData> regional_data;
    };

    class EducationLevelTracker {
    private:
        uint32_t get_region_for_tile(TileCoord coord) const {
            return static_cast<uint32_t>(coord.x / 100);
        }

        /**
         * @brief Infers highest qualification from the citizen's maximum accumulated skill.
         */
        Qualification determine_qualification(const Population::EducationComponent& edu) const {
            float max_skill = 0.0f;
            for (float skill : edu.skill_levels) {
                if (skill > max_skill) max_skill = skill;
            }

            if (max_skill >= 6.0f) return Qualification::Tertiary;
            if (max_skill >= 3.0f) return Qualification::Secondary;
            if (max_skill >= 1.0f) return Qualification::Primary;
            return Qualification::None;
        }

    public:
        /**
         * @brief Scans the working-age population monthly to generate education histograms and productivity modifiers.
         */
        void update(ECSWorld& world, EventPublisher& publisher) {
            auto view = world.registry.view<
                PositionComponent,
                Population::AgeLifecycleComponent,
                Population::EducationComponent
            >();

            EducationDemographicsEvent report;

            // --- 1. Bucket and Count ---
            for (auto raw_id : view) {
                const auto& age = view.get<Population::AgeLifecycleComponent>(raw_id);

                // We strictly only count Working Age populations (Adults)
                if (age.stage != Population::LifeStage::Adult) {
                    continue;
                }

                const auto& pos = view.get<PositionComponent>(raw_id);
                const auto& edu = view.get<Population::EducationComponent>(raw_id);

                uint32_t region = get_region_for_tile(pos.coord);
                Qualification qual = determine_qualification(edu);

                report.regional_data[region].working_age_population++;
                report.regional_data[region].edu_histogram[static_cast<size_t>(qual)]++;
            }

            // --- 2. Calculate Macroeconomic Ratios & Modifiers ---
            for (auto& [region, data] : report.regional_data) {
                if (data.working_age_population > 0) {
                    uint32_t tertiary_count = data.edu_histogram[static_cast<size_t>(Qualification::Tertiary)];
                    data.tertiary_ratio = static_cast<float>(tertiary_count) / static_cast<float>(data.working_age_population);

                    // Formula: 1.0 + (tertiary_ratio - 0.15) * 2.0
                    // A ratio of 0.15 is the baseline. We clamp the minimum at 0.8 so total uneducated regions aren't totally destroyed.
                    float bonus = 1.0f + ((data.tertiary_ratio - 0.15f) * 2.0f);
                    data.regional_productivity_bonus = std::max(0.8f, bonus);
                }
            }

            // --- 3. Broadcast ---
            publisher.publish(report);
        }
    };
}