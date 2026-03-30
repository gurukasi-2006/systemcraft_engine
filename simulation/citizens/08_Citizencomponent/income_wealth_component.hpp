#pragma once

#include <cstdint>
#include <array>

#include "../../../core/04_Types/fixed_point.hpp"
#include "../../../core/04_Types/economic_constants.hpp"

/**
 * @file income_wealth_component.hpp
 * @brief Subsystem 95: Tracks personal finances, strict expenditure orders, and debt accumulation.
 */

namespace Population {

    /**
     * @enum ExpenseCategory
     * @brief The core pillars of citizen expenditure.
     */
    enum class ExpenseCategory : uint8_t {
        Housing = 0,
        Food = 1,
        Transport = 2,
        Leisure = 3,
        COUNT
    };

    /**
     * @struct IncomeWealthComponent
     * @brief The financial ledger of a single citizen. Forms the basis of the Gini coefficient.
     */
    struct IncomeWealthComponent {
        Fixed32 monthly_income{0.0f};
        Fixed32 savings{0.0f};
        Fixed32 debt{0.0f};

        // Tracks exactly how much was spent on each category this month
        std::array<Fixed32, static_cast<size_t>(ExpenseCategory::COUNT)> expenditure_breakdown{};

        /**
         * @brief Processes one month of income, taxes, debt interest, and strict expenditures.
         * @param gross_income Income earned from employment or welfare.
         * @param rent_cost The rigid cost of their current housing.
         * @param target_food The cost of a fully satisfying diet.
         * @param target_transport The cost of commuting to their job.
         * @param target_leisure The cost of their desired entertainment.
         * @param leisure_threshold Minimum savings required before they will spend on leisure (default 100.0).
         * @return The actual amount of food purchased (used by the Health System to calculate starvation).
         */
        inline Fixed32 process_monthly_finances(
            Fixed32 gross_income,
            Fixed32 rent_cost,
            Fixed32 target_food,
            Fixed32 target_transport,
            Fixed32 target_leisure,
            Fixed32 leisure_threshold = Fixed32(100.0f)
        ) {
            monthly_income = gross_income;

            // 1. Taxation & Net Income
            Fixed32 income_tax = gross_income * Fixed32(Tax::BASE_INCOME_TAX_RATE);
            Fixed32 net_income = gross_income - income_tax;

            // 2. Accumulate Debt Interest (Old Debt * (1 + Annual_Rate / 12))
            Fixed32 monthly_interest_rate = Fixed32(Debt::SOVEREIGN_DEBT_INTEREST_BASE) / Fixed32(12.0f);
            debt = debt * (Fixed32(1.0f) + monthly_interest_rate);

            // 3. Deposit Income
            savings = savings + net_income;

            // 4. Strict Deduction Order (The Poverty Trap Logic)

            // A. Housing (Eviction prevention takes priority. Can cause debt.)
            if (savings >= rent_cost) {
                savings = savings - rent_cost;
                expenditure_breakdown[static_cast<size_t>(ExpenseCategory::Housing)] = rent_cost;
            } else {
                Fixed32 shortfall = rent_cost - savings;
                expenditure_breakdown[static_cast<size_t>(ExpenseCategory::Housing)] = rent_cost;
                debt = debt + shortfall; // Citizen takes on debt to avoid homelessness!
                savings = Fixed32(0.0f);
            }

            // B. Food (Cannot take debt for food. You buy what you can afford.)
            if (savings >= target_food) {
                savings = savings - target_food;
                expenditure_breakdown[static_cast<size_t>(ExpenseCategory::Food)] = target_food;
            } else {
                expenditure_breakdown[static_cast<size_t>(ExpenseCategory::Food)] = savings;
                savings = Fixed32(0.0f);
            }

            // C. Transport
            if (savings >= target_transport) {
                savings = savings - target_transport;
                expenditure_breakdown[static_cast<size_t>(ExpenseCategory::Transport)] = target_transport;
            } else {
                expenditure_breakdown[static_cast<size_t>(ExpenseCategory::Transport)] = savings;
                savings = Fixed32(0.0f);
            }

            // D. Leisure (Only purchased if savings remain comfortably above threshold)
            if (savings > leisure_threshold && savings >= target_leisure) {
                savings = savings - target_leisure;
                expenditure_breakdown[static_cast<size_t>(ExpenseCategory::Leisure)] = target_leisure;
            } else {
                expenditure_breakdown[static_cast<size_t>(ExpenseCategory::Leisure)] = Fixed32(0.0f);
            }

            // Return the actual food budget secured, so the simulation knows if they starve this month
            return expenditure_breakdown[static_cast<size_t>(ExpenseCategory::Food)];
        }
    };
}