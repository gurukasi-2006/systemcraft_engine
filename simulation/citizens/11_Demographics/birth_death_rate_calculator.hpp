#pragma once

#include <cstdint>
#include <cmath>
#include <limits>

#include "../../../core/03_Event_Bus/subscriber_registry.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/time_constants.hpp"

// Assuming these were defined in Subsystem 105 (Birth) and 106 (Death)
#include "../09_CitizenSpawner/birth_rate_controller.hpp"
#include "../09_CitizenSpawner/death_handler.hpp"

/**
 * @file birth_death_rate_calculator.hpp
 * @brief Subsystem 130: Computes annual CBR, CDR, and NIR from event counters.
 */

namespace Demographics {

    /**
     * @struct DemographicRatesReportEvent
     * @brief The annual statistical packet sent to the UI and Stats Bureau.
     */
    struct DemographicRatesReportEvent {
        float crude_birth_rate{0.0f};      // Births per 1000 citizens
        float crude_death_rate{0.0f};      // Deaths per 1000 citizens
        float natural_increase_rate{0.0f}; // Growth per 1000 citizens
        float doubling_time_years{0.0f};   // Projected years to double population
    };

    // Forward declarations in case the headers don't define the exact structs
    #ifndef CITIZEN_SPAWNED_EVENT_HPP
    #define CITIZEN_SPAWNED_EVENT_HPP
    struct CitizenSpawnedEvent { EntityID citizen; uint32_t region_id; };
    #endif

    class BirthDeathRateCalculator {
    private:
        uint32_t births_this_year_{0};
        uint32_t deaths_this_year_{0};

    public:
        /**
         * @brief Hooks into the Event Bus to count births and deaths in O(1) time.
         */
        BirthDeathRateCalculator(SubscriberRegistry& registry) {
            registry.subscribe<CitizenSpawnedEvent>([this](const CitizenSpawnedEvent&) {
                births_this_year_++;
            });

            registry.subscribe<CitizenDiedEvent>([this](const CitizenDiedEvent&) {
                deaths_this_year_++;
            });
        }

        /**
         * @brief Evaluates the counters annually, calculates demographics, and resets.
         * @param publisher The central event bus publisher.
         * @param current_tick The absolute simulation time.
         * @param total_population Provided by Subsystem 128 (PopulationCounter).
         */
        void update(EventPublisher& publisher, uint64_t current_tick, uint32_t total_population) {
            // Execute ONLY precisely on the new year rollover
            if (current_tick == 0 || current_tick % TimeConstants::TICKS_PER_YEAR != 0) {
                return;
            }

            DemographicRatesReportEvent report;

            if (total_population > 0) {
                float pop_float = static_cast<float>(total_population);

                // --- 1. Crude Rates (Per 1000 Citizens) ---
                report.crude_birth_rate = (static_cast<float>(births_this_year_) / pop_float) * 1000.0f;
                report.crude_death_rate = (static_cast<float>(deaths_this_year_) / pop_float) * 1000.0f;

                // --- 2. Natural Increase Rate ---
                report.natural_increase_rate = report.crude_birth_rate - report.crude_death_rate;

                // --- 3. Doubling Time (Rule of 70) ---
                if (report.natural_increase_rate > 0.0f) {
                    // NIR is per 1000. To get percentage growth rate, divide by 10.
                    float percentage_growth = report.natural_increase_rate / 10.0f;
                    report.doubling_time_years = 70.0f / percentage_growth;
                } else if (report.natural_increase_rate < 0.0f) {
                    // Population is shrinking; doubling time is mathematically undefined/infinite
                    report.doubling_time_years = std::numeric_limits<float>::infinity();
                } else {
                    // Perfect stagnation
                    report.doubling_time_years = 0.0f;
                }
            }

            // --- 4. Fire Macro-Economic Event ---
            publisher.publish(report);

            // --- 5. Reset Annual Counters ---
            births_this_year_ = 0;
            deaths_this_year_ = 0;
        }
    };
}