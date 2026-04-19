#pragma once

#include <entt/entt.hpp>
#include <cmath>
#include <vector>
#include <array>
#include <algorithm>
#include <string_view>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/time_constants.hpp"

#include "../08_Citizencomponent/health_component.hpp"
#include "../../../data/tables/disease_tables.hpp" // <-- INJECTED YOUR UPLOADED FILE!

/**
 * @file health_statistics_aggregator.hpp
 * @brief Subsystem 137: Computes population health metrics and tracks epidemiological outbreaks.
 */

namespace Demographics {

    /**
     * @struct HealthStatisticsReportEvent
     * @brief Monthly macroeconomic packet containing population health and active outbreak data.
     */
    struct HealthStatisticsReportEvent {
        float mean_health{0.0f};
        float disease_prevalence{0.0f}; // Percentage of population with active diseases (0.0 to 1.0)
        float life_expectancy_approx{0.0f};

        // Epidemiological tracking
        std::vector<std::string_view> top_spreading_diseases;
    };

    class HealthStatisticsAggregator {
    public:
        /**
         * @brief Scans all citizens, computes health averages, and identifies the most prevalent diseases.
         */
        void update(ECSWorld& world, EventPublisher& publisher, uint64_t current_tick) {
            constexpr uint64_t MONTH_TICKS = TimeConstants::TICKS_PER_YEAR / 12ULL;

            if (current_tick == 0 || current_tick % MONTH_TICKS != 0) {
                return;
            }

            auto view = world.registry.view<Population::HealthComponent>();

            double total_health = 0.0;
            uint32_t total_pop = 0;
            uint32_t diseased_pop = 0;

            // Track how many citizens have each specific disease ID (0 to 39)
            std::array<uint32_t, DISEASE_TABLE.size()> disease_histogram{};

            for (auto raw_id : view) {
                const auto& health = view.get<Population::HealthComponent>(raw_id);

                // Fallback adapter for health value
                float current_val = 50.0f;
                if constexpr (requires { health.physical_health; }) {
                    current_val = health.physical_health;
                } else if constexpr (requires { health.physical_health; }) {
                    current_val = health.physical_health;
                }

                total_health += current_val;
                total_pop++;

                // Track Epidemiology
                bool has_disease = false;

                // Assuming active_diseases is a vector of IDs or Enums mapping to your table
                if constexpr (requires { health.active_diseases.empty(); }) {
                    has_disease = !health.active_diseases.empty();
                    for (auto disease_id : health.active_diseases) {
                        // Cast to size_t and safeguard bounds
                        size_t idx = static_cast<size_t>(disease_id);
                        if (idx < disease_histogram.size()) {
                            disease_histogram[idx]++;
                        }
                    }
                }

                if (has_disease) {
                    diseased_pop++;
                }
            }

            if (total_pop > 0) {
                HealthStatisticsReportEvent report;

                report.mean_health = static_cast<float>(total_health / static_cast<double>(total_pop));
                report.disease_prevalence = static_cast<float>(diseased_pop) / static_cast<float>(total_pop);
                report.life_expectancy_approx = (report.mean_health * 0.5f) + 50.0f;

                // --- Find Top 3 Spreading Diseases ---
                struct DiseaseStat {
                    size_t id;
                    uint32_t count;
                };
                std::vector<DiseaseStat> stats;
                for (size_t i = 0; i < disease_histogram.size(); ++i) {
                    if (disease_histogram[i] > 0) {
                        stats.push_back({i, disease_histogram[i]});
                    }
                }

                // Sort descending by count
                std::sort(stats.begin(), stats.end(), [](const DiseaseStat& a, const DiseaseStat& b) {
                    return a.count > b.count;
                });

                // Map Top 3 IDs to their actual names from your table!
                for (size_t i = 0; i < std::min<size_t>(3, stats.size()); ++i) {
                    report.top_spreading_diseases.push_back(DISEASE_TABLE[stats[i].id].name);
                }

                publisher.publish(report);
            }
        }
    };
}