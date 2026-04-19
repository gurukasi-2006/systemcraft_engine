#pragma once

#include <entt/entt.hpp>
#include <vector>
#include <algorithm>
#include <numeric>
#include <array>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/time_constants.hpp"
#include "../../../core/04_Types/economic_constants.hpp"

#include "../08_Citizencomponent/employment_component.hpp"

/**
 * @file income_distribution_analyser.hpp
 * @brief Subsystem 134: Calculates the Gini coefficient and income quintiles, triggering political instability if inequality is high.
 */

namespace Demographics {

    /**
     * @struct GiniReportEvent
     * @brief Monthly macroeconomic packet containing the Gini coefficient and its political impact.
     */
    struct GiniReportEvent {
        float gini_coefficient{0.0f};
        float instability_delta{0.0f}; // The monthly political penalty
        std::array<float, 5> quintile_shares{}; // Percentage of total income held by each 20% block
    };

    class IncomeDistributionAnalyser {
    public:
        /**
         * @brief Scans all citizen incomes, sorts them, and computes structural inequality.
         */
        void update(ECSWorld& world, EventPublisher& publisher, uint64_t current_tick) {
            constexpr uint64_t MONTH_TICKS = TimeConstants::TICKS_PER_YEAR / 12ULL;

            // Execute only on the monthly rollover tick
            if (current_tick == 0 || current_tick % MONTH_TICKS != 0) {
                return;
            }

            auto view = world.registry.view<Population::EmploymentComponent>();

            std::vector<float> incomes;
            incomes.reserve(view.size());

            for (auto raw_id : view) {
                const auto& emp = view.get<Population::EmploymentComponent>(raw_id);
                // We consider all citizens. Unemployed will naturally have 0.0f wage.
                incomes.push_back(emp.wage.toFloat());
            }

            size_t n = incomes.size();
            if (n == 0) return;

            // --- 1. Sort Ascending for Gini Math ---
            std::sort(incomes.begin(), incomes.end());

            // --- 2. Calculate Exact Gini Coefficient ---
            double sum_incomes = 0.0;
            double weighted_sum = 0.0;
            double dn = static_cast<double>(n);

            for (size_t i = 0; i < n; ++i) {
                double inc = incomes[i];
                sum_incomes += inc;

                // Formula: sum((2 * rank - n - 1) * income)
                // Rank is 1-indexed, so rank = i + 1
                double rank = static_cast<double>(i + 1);
                weighted_sum += ((2.0 * rank) - dn - 1.0) * inc;
            }

            GiniReportEvent report;

            if (sum_incomes > 0.0) {
                report.gini_coefficient = static_cast<float>(weighted_sum / (dn * sum_incomes));

                // Calculate Quintile Shares (for UI charts)
                size_t chunk_size = n / 5;
                if (chunk_size > 0) {
                    for (size_t q = 0; q < 5; ++q) {
                        double chunk_sum = 0.0;
                        size_t start = q * chunk_size;
                        // For the 5th quintile, grab any remainders
                        size_t end = (q == 4) ? n : start + chunk_size;

                        for (size_t i = start; i < end; ++i) {
                            chunk_sum += incomes[i];
                        }
                        report.quintile_shares[q] = static_cast<float>(chunk_sum / sum_incomes);
                    }
                }
            }

            // --- 3. Evaluate Political Instability ---
            constexpr float GINI_INSTABILITY_COEFFICIENT = 0.80f;

            if (report.gini_coefficient >= 0.60f) {
                // Extreme (pre-revolution): Overrides the linear formula with the hard +8.0 penalty
                report.instability_delta = 8.0f;
            }
            else if (report.gini_coefficient > 0.35f) {
                // High to Severe Inequality Formula: (Gini - 0.30) / 0.10 * 0.80
                report.instability_delta = ((report.gini_coefficient - 0.30f) / 0.10f) * GINI_INSTABILITY_COEFFICIENT;
            }
            else {
                // Very equal to Moderate (Scandinavia): Safe zone
                report.instability_delta = 0.0f;
            }

            // --- 4. Broadcast ---
            publisher.publish(report);
        }
    };
}