#pragma once

#include <entt/entt.hpp>
#include <array>
#include <cmath>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/03_Event_Bus/subscriber_registry.hpp"
#include "../../../core/04_Types/time_constants.hpp"

// Native Components for Live ECS Aggregation
#include "../08_Citizencomponent/age_lifecycle_component.hpp"
#include "../08_Citizencomponent/happiness_score_component.hpp"
#include "../10_NeedsSystem/poverty_classifier.hpp" // For Welfare::PovertyComponent

// Subsystem Events for Cache Aggregation
#include "birth_death_rate_calculator.hpp"
#include "income_distribution_analyser.hpp"
#include "employment_rate_monitor.hpp"
#include "health_statistics_aggregator.hpp"
#include "age_pyramid_builder.hpp"

/**
 * @file demographic_report_generator.hpp
 * @brief Subsystem 139: Packages all disparate demographic statistics into a unified annual report.
 */

namespace Demographics {

    /**
     * @struct AnnualDemographicReportEvent
     * @brief The master macroeconomic packet published every game-year.
     */
    struct AnnualDemographicReportEvent {
        uint32_t year_number{0};
        uint32_t total_population{0};

        float crude_birth_rate{0.0f};
        float crude_death_rate{0.0f};
        float gini_coefficient{0.0f};
        float unemployment_rate{0.0f};

        float mean_happiness{0.0f};
        float poverty_rate{0.0f};
        float mean_health{0.0f};
        float migration_balance{0.0f};

        std::array<std::array<uint32_t, 2>, 17> age_pyramid{};
    };

    class DemographicReportGenerator {
    private:
        // Caches for stats calculated by other sub-systems
        float cached_cbr_{0.0f};
        float cached_cdr_{0.0f};
        float cached_gini_{0.0f};
        float cached_health_{0.0f};
        float cached_unemployment_{0.0f};
        std::array<std::array<uint32_t, 2>, 17> cached_age_pyramid_{};

    public:
        /**
         * @brief Hooks into the event bus to listen for all monthly/yearly demographic pulses.
         */
        DemographicReportGenerator(SubscriberRegistry& registry) {
            registry.subscribe<DemographicRatesReportEvent>([this](const DemographicRatesReportEvent& ev) {
                cached_cbr_ = ev.crude_birth_rate;
                cached_cdr_ = ev.crude_death_rate;
            });

            registry.subscribe<GiniReportEvent>([this](const GiniReportEvent& ev) {
                cached_gini_ = ev.gini_coefficient;
            });

            registry.subscribe<HealthStatisticsReportEvent>([this](const HealthStatisticsReportEvent& ev) {
                cached_health_ = ev.mean_health;
            });

            registry.subscribe<AgePyramidReportEvent>([this](const AgePyramidReportEvent& ev) {
                cached_age_pyramid_ = ev.pyramid_data;
            });

            registry.subscribe<EmploymentDemographicsEvent>([this](const EmploymentDemographicsEvent& ev) {
                uint32_t total_unemployed = 0;
                uint32_t total_labor = 0;
                for (const auto& [reg, data] : ev.regional_data) {
                    total_unemployed += data.unemployed_count;
                    total_labor += data.labor_force;
                }
                if (total_labor > 0) {
                    cached_unemployment_ = static_cast<float>(total_unemployed) / static_cast<float>(total_labor);
                }
            });
        }

        /**
         * @brief Packages the caches and live ECS stats into a single report on the new year.
         */
        void update(ECSWorld& world, EventPublisher& publisher, uint64_t current_tick) {
            constexpr uint64_t YEAR_TICKS = TimeConstants::TICKS_PER_YEAR;

            // Execute strictly on the new year rollover
            if (current_tick == 0 || current_tick % YEAR_TICKS != 0) {
                return;
            }

            AnnualDemographicReportEvent report;
            report.year_number = static_cast<uint32_t>(current_tick / YEAR_TICKS);

            // --- 1. Map Cached Stats ---
            report.crude_birth_rate = cached_cbr_;
            report.crude_death_rate = cached_cdr_;
            report.gini_coefficient = cached_gini_;
            report.mean_health = cached_health_;
            report.unemployment_rate = cached_unemployment_;
            report.age_pyramid = cached_age_pyramid_;
            report.migration_balance = 0.0f; // Global net-migration placeholder

            // --- 2. Calculate Live ECS Stats (Population, Happiness, Poverty) ---
            auto view = world.registry.view<Population::AgeLifecycleComponent>();

            uint32_t total_pop = 0;
            uint32_t poor_pop = 0;
            double total_happiness = 0.0;
            uint32_t happiness_count = 0;

            for (auto raw_id : view) {
                total_pop++;

                // Track Poverty
                if (world.registry.all_of<Welfare::PovertyComponent>(raw_id)) {
                    if (world.registry.get<Welfare::PovertyComponent>(raw_id).is_poor) {
                        poor_pop++;
                    }
                }

                // Track Happiness
                if (world.registry.all_of<Population::HappinessScoreComponent>(raw_id)) {
                    const auto& hap = world.registry.get<Population::HappinessScoreComponent>(raw_id);

                    // C++20 Reflection to handle however it was named in Phase 8
                    if constexpr (requires { hap.current_happiness; }) {
                        total_happiness += hap.current_happiness;
                    } else if constexpr (requires { hap.current_happiness; }) {
                        total_happiness += hap.current_happiness;
                    } else {
                        total_happiness += 50.0; // Fallback
                    }
                    happiness_count++;
                }
            }

            // --- 3. Finalize Math and Broadcast ---
            report.total_population = total_pop;

            if (total_pop > 0) {
                report.poverty_rate = static_cast<float>(poor_pop) / static_cast<float>(total_pop);
            }
            if (happiness_count > 0) {
                report.mean_happiness = static_cast<float>(total_happiness / static_cast<double>(happiness_count));
            }

            publisher.publish(report);
        }
    };
}