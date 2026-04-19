#pragma once

#include <entt/entt.hpp>
#include <array>
#include <cstdint>
#include <algorithm>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/time_constants.hpp"

#include "../08_Citizencomponent/age_lifecycle_component.hpp"

/**
 * @file age_pyramid_builder.hpp
 * @brief Subsystem 129: Buckets citizens by age and sex, generating demographic advisories.
 */

namespace Demographics {

    enum class BiologicalSex : uint8_t {
        Male = 0,
        Female = 1
    };

    /**
     * @struct BiologicalSexComponent
     * @brief Required for charting the left/right demographic splits of an age pyramid.
     */
    struct BiologicalSexComponent {
        BiologicalSex sex{BiologicalSex::Female};
    };

    /**
     * @struct AgePyramidReportEvent
     * @brief The monthly statistical packet sent to the UI and Stats Bureau.
     */
    struct AgePyramidReportEvent {
        // 17 buckets (0-4, 5-9 ... 80+). Index 0 = Male, Index 1 = Female.
        std::array<std::array<uint32_t, 2>, 17> pyramid_data{};

        uint32_t total_population{0};

        // UI Advisories
        bool youth_bulge_warning{false};    // Too many children, educational strain
        bool aging_workforce_warning{false}; // Too many retirees, pension/labor strain
    };

    class AgePyramidBuilder {
    public:
        /**
         * @brief Scans the population monthly to build the age pyramid and trigger advisories.
         */
        void update(ECSWorld& world, EventPublisher& publisher, uint64_t current_tick) {
            constexpr uint64_t MONTH_TICKS = TimeConstants::TICKS_PER_YEAR / 12ULL;

            // Execute only on the monthly rollover tick
            if (current_tick == 0 || current_tick % MONTH_TICKS != 0) {
                return;
            }

            AgePyramidReportEvent report;

            auto view = world.registry.view<
                Population::AgeLifecycleComponent,
                BiologicalSexComponent
            >();

            uint32_t youth_count = 0;   // Ages 0-19 (Buckets 0-3)
            uint32_t working_count = 0; // Ages 20-64 (Buckets 4-12)
            uint32_t elder_count = 0;   // Ages 65+ (Buckets 13-16)

            for (auto raw_id : view) {
                const auto& age = view.get<Population::AgeLifecycleComponent>(raw_id);
                const auto& bio = view.get<BiologicalSexComponent>(raw_id);

                // Calculate age in years
                uint32_t age_years = static_cast<uint32_t>(age.current_age_ticks / TimeConstants::TICKS_PER_YEAR);

                // Integer division by 5 gives us the 5-year buckets. Clamp at 16 (80+ years old).
                uint32_t bucket = std::min(age_years / 5, 16u);

                // Increment the specific demographic bucket
                report.pyramid_data[bucket][static_cast<size_t>(bio.sex)]++;
                report.total_population++;

                // Track macro-cohorts for UI warnings
                if (bucket <= 3) {
                    youth_count++;
                } else if (bucket <= 12) {
                    working_count++;
                } else {
                    elder_count++;
                }
            }

            if (report.total_population > 0) {
                // --- Advisory Trigger 1: Youth Bulge ---
                // If under-20s make up more than 40% of the entire population, education will collapse
                float youth_ratio = static_cast<float>(youth_count) / report.total_population;
                if (youth_ratio > 0.40f) {
                    report.youth_bulge_warning = true;
                }

                // --- Advisory Trigger 2: Aging Workforce ---
                // If the "Old-Age Dependency Ratio" exceeds 35% (Elders relative to Workers), labor falls short
                if (working_count > 0) {
                    float dependency_ratio = static_cast<float>(elder_count) / working_count;
                    if (dependency_ratio > 0.35f) {
                        report.aging_workforce_warning = true;
                    }
                } else if (elder_count > 0) {
                    // Absolute labor collapse (Elders exist but zero workers)
                    report.aging_workforce_warning = true;
                }
            }

            publisher.publish(report);
        }
    };
}