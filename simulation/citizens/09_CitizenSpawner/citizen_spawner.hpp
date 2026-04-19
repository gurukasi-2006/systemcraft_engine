#pragma once

#include <random>
#include <algorithm>
#include <string> // <-- Added for std::string
#include <entt/entt.hpp>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/04_Types/tile_coord.hpp"
#include "../../../core/04_Types/fixed_point.hpp"
#include "../../../core/04_Types/math_utils.hpp"
#include "../../../core/04_Types/time_constants.hpp"

#include "../08_Citizencomponent/age_lifecycle_component.hpp"
#include "../08_Citizencomponent/health_component.hpp"
#include "../08_Citizencomponent/income_wealth_component.hpp"
#include "../08_Citizencomponent/identity_component.hpp"

// Assuming PositionComponent is defined globally or locally as such
struct PositionComponent { TileCoord coord; };

/**
 * @file citizen_spawner.hpp
 * @brief Subsystem 102: Entity Factory for citizens. Handles statistical seeding for realistic demographics.
 */

class CitizenSpawner {
public:
    static EntityID spawn_game_start_citizen(ECSWorld& world, TileCoord birth_tile, Fixed32 median_wage, std::mt19937& rng) {
        EntityID citizen = world.entity_manager.createEntity();
        entt::entity raw_id = static_cast<entt::entity>(citizen.raw_id);

        // --- 1. AGE GENERATION (LogNormal) ---
        std::lognormal_distribution<float> age_dist(3.4f, 0.5f);
        float age_years = MathUtils::clamp(age_dist(rng), 0.0f, 80.0f);

        uint64_t age_ticks = static_cast<uint64_t>(age_years * TimeConstants::TICKS_PER_YEAR);

        Population::LifeStage stage = Population::LifeStage::Child;
        if (age_years >= 65.0f) stage = Population::LifeStage::Elder;
        else if (age_years >= 18.0f) stage = Population::LifeStage::Adult;

        std::normal_distribution<float> lifespan_dist(75.0f, 10.0f);
        uint32_t expected_lifespan = MathUtils::clamp(static_cast<uint32_t>(lifespan_dist(rng)), 45u, 100u);

        world.registry.emplace<Population::AgeLifecycleComponent>(raw_id, age_ticks, stage, expected_lifespan);

        // --- 2. HEALTH GENERATION ---
        std::normal_distribution<float> health_variance_dist(0.0f, 8.0f);
        float base_health = 100.0f - (age_years * 0.3f);
        float final_health = MathUtils::clamp(base_health + health_variance_dist(rng), 20.0f, 100.0f);

        world.registry.emplace<Population::HealthComponent>(raw_id, final_health);

        // --- 3. WEALTH/SAVINGS GENERATION ---
        std::normal_distribution<float> savings_dist(2.0f, 1.2f);
        float savings_multiplier = std::max(0.0f, savings_dist(rng));
        Fixed32 starting_savings = median_wage * Fixed32(savings_multiplier);

        world.registry.emplace<Population::IncomeWealthComponent>(raw_id, starting_savings, median_wage);

        // --- 4. POSITION & IDENTITY ---
        world.registry.emplace<PositionComponent>(raw_id, birth_tile);

        // Generate the 4 missing pieces for IdentityComponent
        uint32_t id = generate_random_id(rng);
        std::string name = "Citizen_" + std::to_string(id);
        Population::BiologicalSex sex = (id % 2 == 0) ? Population::BiologicalSex::Male : Population::BiologicalSex::Female;

        world.registry.emplace<Population::IdentityComponent>(raw_id, id, name, 0ULL, sex);

        return citizen;
    }

    static EntityID spawn_newborn(ECSWorld& world, TileCoord birth_tile, std::mt19937& rng) {
        EntityID citizen = world.entity_manager.createEntity();
        entt::entity raw_id = static_cast<entt::entity>(citizen.raw_id);

        std::normal_distribution<float> lifespan_dist(75.0f, 10.0f);
        uint32_t expected_lifespan = MathUtils::clamp(static_cast<uint32_t>(lifespan_dist(rng)), 45u, 100u);

        world.registry.emplace<Population::AgeLifecycleComponent>(raw_id, 0ULL, Population::LifeStage::Child, expected_lifespan);

        std::normal_distribution<float> infant_health_dist(95.0f, 10.0f);
        float final_health = MathUtils::clamp(infant_health_dist(rng), 10.0f, 100.0f);
        world.registry.emplace<Population::HealthComponent>(raw_id, final_health);

        world.registry.emplace<Population::IncomeWealthComponent>(raw_id, Fixed32(0), Fixed32(0));

        world.registry.emplace<PositionComponent>(raw_id, birth_tile);

        // Generate the 4 missing pieces for IdentityComponent
        uint32_t id = generate_random_id(rng);
        std::string name = "Citizen_" + std::to_string(id);
        Population::BiologicalSex sex = (id % 2 == 0) ? Population::BiologicalSex::Male : Population::BiologicalSex::Female;

        world.registry.emplace<Population::IdentityComponent>(raw_id, id, name, 0ULL, sex);

        return citizen;
    }

private:
    static uint32_t generate_random_id(std::mt19937& rng) {
        std::uniform_int_distribution<uint32_t> id_dist(100000, 999999);
        return id_dist(rng);
    }
};