#pragma once

#include <vector>
#include <span>
#include <entt/entt.hpp>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/04_Types/tile_coord.hpp"
#include "../../../core/04_Types/entity_id.hpp"

#include "../08_Citizencomponent/employment_component.hpp"
#include "../08_Citizencomponent/age_lifecycle_component.hpp"
#include "../08_Citizencomponent/income_wealth_component.hpp"
#include "../09_CitizenSpawner/citizen_spawner.hpp" // For PositionComponent

/**
 * @file citizen_query_api.hpp
 * @brief Subsystem 113: High-performance, zero-allocation ECS views and queries.
 */

class CitizenQueryAPI {
private:
    // --- Pre-allocated Buffers ---
    // These vectors grow once and retain their capacity. By clearing them instead
    // of recreating them, we completely eliminate heap allocations on the hot path!
    std::vector<EntityID> tile_query_buffer_;
    std::vector<EntityID> employer_query_buffer_;

public:
    /**
     * @brief Finds all citizens standing on a specific TileCoord.
     * @return A lightweight C++20 span pointing to the internal buffer.
     * @warning The span is only valid until the next time this function is called!
     */
    std::span<EntityID> getCitizensInTile(ECSWorld& world, TileCoord coord) {
        tile_query_buffer_.clear(); // O(1) operation, does not free memory capacity

        auto view = world.registry.view<PositionComponent>();
        for (auto raw_id : view) {
            if (view.get<PositionComponent>(raw_id).coord == coord) {
                tile_query_buffer_.push_back(EntityID{static_cast<uint32_t>(raw_id)});
            }
        }

        return std::span<EntityID>(tile_query_buffer_);
    }

    /**
     * @brief Retrieves all citizens working for a specific Employer ID.
     */
    const std::vector<EntityID>& getCitizensByEmployer(ECSWorld& world, EntityID employer) {
        employer_query_buffer_.clear();

        auto view = world.registry.view<Population::EmploymentComponent>();
        for (auto raw_id : view) {
            if (view.get<Population::EmploymentComponent>(raw_id).employer.raw_id == employer.raw_id) {
                employer_query_buffer_.push_back(EntityID{static_cast<uint32_t>(raw_id)});
            }
        }

        return employer_query_buffer_;
    }

    /**
     * @brief Returns a raw EnTT view of all citizens capable of employment.
     * @details Caller iterates this view and checks `status == Unemployed`.
     * Returning the raw ECS view is the fastest possible iteration method in EnTT.
     */
    auto getUnemployedCitizens(ECSWorld& world) {
        return world.registry.view<Population::EmploymentComponent>();
    }

    /**
     * @brief Returns a raw EnTT view combining Age and Wealth for complex poverty checks.
     * @details Caller iterates this view and checks `stage == Elder` and `savings == 0 && debt > 0`.
     */
    auto getElderlyPoor(ECSWorld& world) {
        return world.registry.view<Population::AgeLifecycleComponent, Population::IncomeWealthComponent>();
    }
};