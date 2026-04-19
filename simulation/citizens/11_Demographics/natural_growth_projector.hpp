#pragma once

#include <cstdint>
#include <cmath>
#include <array>

#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/time_constants.hpp"

/**
 * @file natural_growth_projector.hpp
 * @brief Subsystem 131: Projects population growth across 5 scenarios using exponential models.
 */

namespace Demographics {

    /**
     * @enum ProjectionScenario
     * @brief The 5 distinct future timelines calculated for the Fan Chart.
     */
    enum class ProjectionScenario : uint8_t {
        Optimistic = 0,  // r + 0.5%
        Base,            // Standard CBR - CDR + Net Migration
        Pessimistic,     // r - 0.5%
        ZeroMigration,   // Internal natural growth only
        Crisis,          // Base CBR, but CDR is doubled
        COUNT
    };

    /**
     * @struct PopulationProjectionEvent
     * @brief Data packet containing a multi-year fan chart matrix.
     * @details Array dimensions: [5 Scenarios][N Years]. Defaults to a 10-year projection.
     */
    template <size_t N_YEARS = 10>
    struct PopulationProjectionEvent {
        std::array<std::array<uint32_t, N_YEARS>, static_cast<size_t>(ProjectionScenario::COUNT)> fan_chart_data{};
        uint32_t baseline_population{0};
    };

    class NaturalGrowthProjector {
    public:
        /**
         * @brief Calculates the exponential population growth matrix (Fan Chart) and publishes it.
         * @param publisher The global event bus to send the chart to the UI.
         * @param current_tick Absolute simulation time (ensures it only runs annually).
         * @param current_pop The live P0 population count.
         * @param cbr Crude Birth Rate (per 1000).
         * @param cdr Crude Death Rate (per 1000).
         * @param net_migration_rate Emigration/Immigration modifier (defaults to 0.0f).
         */
        void update(EventPublisher& publisher, uint64_t current_tick, uint32_t current_pop,
                    float cbr, float cdr, float net_migration_rate = 0.0f) {

            // Execute ONLY precisely on the new year rollover to match Subsystem 130
            if (current_tick == 0 || current_tick % TimeConstants::TICKS_PER_YEAR != 0) {
                return;
            }

            PopulationProjectionEvent<10> report;
            report.baseline_population = current_pop;

            if (current_pop > 0) {
                // --- 1. Calculate Scenario Growth Rates (r) ---
                float base_r   = ((cbr - cdr) / 1000.0f) + net_migration_rate;
                float opt_r    = base_r + 0.005f; // +0.5%
                float pess_r   = base_r - 0.005f; // -0.5%
                float zm_r     = (cbr - cdr) / 1000.0f;
                float crisis_r = ((cbr - (cdr * 2.0f)) / 1000.0f) + net_migration_rate;

                std::array<float, 5> rates = {opt_r, base_r, pess_r, zm_r, crisis_r};

                // --- 2. Build the Exponential Fan Chart ---
                float p0 = static_cast<float>(current_pop);

                for (size_t scenario = 0; scenario < static_cast<size_t>(ProjectionScenario::COUNT); ++scenario) {
                    float r = rates[scenario];

                    for (size_t t = 0; t < 10; ++t) {
                        // Formula: P(t) = P0 * exp(r * t)
                        // Note: t+1 because array index 0 represents Year 1 of the projection
                        float time_years = static_cast<float>(t + 1);
                        float projected_pop = p0 * std::exp(r * time_years);

                        report.fan_chart_data[scenario][t] = static_cast<uint32_t>(projected_pop);
                    }
                }
            }

            // --- 3. Publish to the UI / Advisory Panel ---
            publisher.publish(report);
        }
    };
}