#pragma once

#include <entt/entt.hpp>
#include <algorithm>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/fixed_point.hpp"

#include "../08_Citizencomponent/income_wealth_component.hpp"
#include "../08_Citizencomponent/employment_component.hpp"
#include "../08_Citizencomponent/health_component.hpp"
#include "../09_CitizenSpawner/commute_resolver.hpp" // For CommuteComponent

/**
 * @file income_processor.hpp
 * @brief Subsystem 109: Executes the strict monthly financial settlement.
 * @details The rigid deduction order (Rent -> Food -> Transport) organically produces
 * a "poverty trap" where citizens sacrifice health (food) to avoid homelessness (rent).
 */

struct RentDefaultedEvent {
    EntityID citizen;
    Fixed32 shortfall;
};

class IncomeProcessor {
public:
    /**
     * @brief Processes the monthly financial settlement for all employed citizens.
     * @param world The ECS master world.
     * @param publisher Event bus to broadcast financial crises (defaults).
     * @param tax_rate Macroeconomic income tax rate (default 20%).
     * @param monthly_food_cost Regional cost of a standard caloric diet.
     * @param working_days Number of billable days in the month.
     */
    void process_month(ECSWorld& world, EventPublisher& publisher, float tax_rate = 0.20f, float monthly_food_cost = 300.0f, int working_days = 20) {

        auto view = world.registry.view<
            Population::IncomeWealthComponent,
            Population::EmploymentComponent,
            Population::CommuteComponent
        >();

        Fixed32 base_rent(600.0f); // Default rent if HousingComponent is not queried

        for (auto raw_id : view) {
            auto& wealth = view.get<Population::IncomeWealthComponent>(raw_id);
            auto& emp = view.get<Population::EmploymentComponent>(raw_id);
            auto& commute = view.get<Population::CommuteComponent>(raw_id);
            EntityID citizen{static_cast<uint32_t>(raw_id)};

            if (emp.status != Population::EmploymentStatus::Employed) {
                continue; // Skip unemployed for now (Handled by Welfare System later)
            }

            // --- 0. Gross Income Calculation ---
            float health = 100.0f;
            if (world.registry.all_of<Population::HealthComponent>(raw_id)) {
                health = world.registry.get<Population::HealthComponent>(raw_id).physical_health;
            }

            // Effective hours accounts for sick days!
            float effective_hours = emp.calculate_effective_hours(health, true);
            Fixed32 gross = emp.calculate_monthly_credit(effective_hours, working_days);

            // --- 1. Taxation ---
            Fixed32 tax = gross * Fixed32(tax_rate);
            Fixed32 net = gross - tax;
            wealth.savings = wealth.savings + net;

            // --- 2. Rent (Housing Priority) ---
            if (wealth.savings >= base_rent) {
                wealth.savings = wealth.savings - base_rent;
            } else {
                Fixed32 shortfall = base_rent - wealth.savings;
                // We assume IncomeWealthComponent has a 'debt' field per your specs!
                wealth.debt = wealth.debt + shortfall;
                wealth.savings = Fixed32(0.0f);

                publisher.publish(RentDefaultedEvent{citizen, shortfall});
            }

            // --- 3. Food (The Health Trap) ---
            Fixed32 food_budget(monthly_food_cost);
            float food_access = 1.0f;

            if (wealth.savings >= food_budget) {
                wealth.savings = wealth.savings - food_budget;
            } else {
                food_access = wealth.savings.toFloat() / food_budget.toFloat();
                wealth.savings = Fixed32(0.0f);
                // Note: The Needs System will read this food_access modifier and decay health!
            }

            // --- 4. Transport ---
            Fixed32 transport_cost(commute.daily_cost * static_cast<float>(working_days));
            if (wealth.savings >= transport_cost) {
                wealth.savings = wealth.savings - transport_cost;
            } else {
                wealth.debt = wealth.debt + (transport_cost - wealth.savings);
                wealth.savings = Fixed32(0.0f);
            }

            // --- 5. Discretionary ---
            Fixed32 leisure_spend(150.0f);
            if (wealth.savings > (base_rent * Fixed32(2.0f))) {
                wealth.savings = wealth.savings - leisure_spend;
            }

            // --- 6. Debt Compounding ---
            // 4% annual interest, compounded monthly -> (1 + 0.04/12)
            if (wealth.debt > Fixed32(0.0f)) {
                float current_debt = wealth.debt.toFloat();
                current_debt *= (1.0f + (0.04f / 12.0f));
                wealth.debt = Fixed32(current_debt);
            }
        }
    }
};