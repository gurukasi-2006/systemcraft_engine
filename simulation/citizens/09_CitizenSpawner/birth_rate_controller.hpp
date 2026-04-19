#pragma once

#include <vector>
#include <random>
#include <algorithm>
#include <entt/entt.hpp>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/04_Types/time_constants.hpp"
#include "../../../core/04_Types/math_utils.hpp"

#include "../08_Citizencomponent/age_lifecycle_component.hpp"
#include "../08_Citizencomponent/identity_component.hpp"
#include "../09_CitizenSpawner/citizen_spawner.hpp"

/**
 * @file birth_rate_controller.hpp
 * @brief Subsystem 105: Calculates and spawns new births per tick based on macro-economic conditions.
 */

class BirthRateController {
private:
    // Accumulates fractional births across ticks (standard in macro-simulations)
    float fractional_births_{0.0f};

public:
    /**
     * @brief Evaluates national fertility and spawns newborns based on economic pressure and housing.
     * @param world The ECS world containing the population.
     * @param median_income The current national median wage.
     * @param poverty_line The calculated poverty threshold.
     * @param vacant_units Total empty housing units in the nation/region.
     * @param rng Deterministic random number generator.
     */
    void update(ECSWorld& world, float median_income, float poverty_line, int32_t vacant_units, std::mt19937& rng) {
        auto view = world.registry.view<Population::AgeLifecycleComponent, Population::IdentityComponent, PositionComponent>();

        std::vector<entt::entity> eligible_women;

        // 1. Identify the fertile population (Female, Age 18-45)
        for (auto raw_id : view) {
            auto& age_comp = view.get<Population::AgeLifecycleComponent>(raw_id);
            auto& id_comp = view.get<Population::IdentityComponent>(raw_id);

            float age_years = static_cast<float>(age_comp.current_age_ticks) / TimeConstants::TICKS_PER_YEAR;

            if (id_comp.sex == Population::BiologicalSex::Female && age_years >= 18.0f && age_years <= 45.0f) {
                eligible_women.push_back(raw_id);
            }
        }

        size_t num_women = eligible_women.size();
        if (num_women == 0) return; // No eligible mothers, no births

        // 2. Calculate Environmental Modifiers
        // Economic Mod: Severe depression reduces fertility to 30%. High prosperity doubles it.
        float economic_mod = std::clamp(median_income / poverty_line, 0.3f, 2.0f);

        // Housing Mod: Scarcity bottlenecks births down to 10%. Surplus boosts to 150%.
        float housing_mod = std::clamp(static_cast<float>(vacant_units) / static_cast<float>(num_women), 0.1f, 1.5f);

        // 3. Compute Actuarial Fertility Rate
        // 2.1 lifetime replacement rate distributed across 27 fertile years (18 to 45)
        float base_annual_fertility = 2.1f / 27.0f;
        float fertility_rate = base_annual_fertility * economic_mod * housing_mod;

        // 4. Convert to Per-Tick Accumulation
        float births_per_tick = (static_cast<float>(num_women) * fertility_rate) / static_cast<float>(TimeConstants::TICKS_PER_YEAR);
        fractional_births_ += births_per_tick;

        // 5. Spawn Whole Entities
        while (fractional_births_ >= 1.0f) {
            // Randomly select a mother to inherit her geographical starting location
            std::uniform_int_distribution<size_t> dist(0, num_women - 1);
            entt::entity mother_id = eligible_women[dist(rng)];
            auto& mother_pos = view.get<PositionComponent>(mother_id);

            // Trigger Subsystem 102 to actually create the ECS baby!
            CitizenSpawner::spawn_newborn(world, mother_pos.coord, rng);

            fractional_births_ -= 1.0f;
        }
    }
};