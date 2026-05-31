#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/entity_id.hpp"
#include "../../../core/04_Types/tile_coord.hpp"

// Housing Components
#include "../12_HousingComponent/structural_quality_component.hpp"
#include "../12_HousingComponent/building_identity_component.hpp"

/**
 * @file building_maintenance_scheduler.hpp
 * @brief Subsystem 162: Degrades structural quality and dispatches contractor entities for repairs.
 */

// --- Safe Mock for Phase 9 (Labour Market) ---
#ifndef CONTRACTOR_COMPONENT_HPP
#define CONTRACTOR_COMPONENT_HPP
namespace Labour {
    struct ContractorComponent {
        uint32_t skill_level{10}; // 0 to 20
        TileCoord location{0, 0};
        bool is_available{true};
    };
}
#endif

namespace Housing {

    enum class MaintenanceUrgency : uint8_t {
        NORMAL = 0,
        HIGH
    };

    // --- Events ---

    struct WorkOrderIssuedEvent {
        EntityID building_id;
        EntityID contractor_id;
        MaintenanceUrgency urgency;
    };

    struct MaintenanceCompletedEvent {
        EntityID building_id;
        EntityID contractor_id;
        float restored_quality;
    };

    /**
     * @struct MaintenanceStateComponent
     * @brief Tracks if a building is currently waiting for a repair crew.
     */
    struct MaintenanceStateComponent {
        bool has_active_work_order{false};
        EntityID assigned_contractor{0};
    };

    /**
     * @class BuildingMaintenanceScheduler
     * @brief Ticks building decay and orchestrates contractor dispatch.
     */
    class BuildingMaintenanceScheduler {
    private:
        uint32_t get_chebyshev_distance(TileCoord a, TileCoord b) const {
            return static_cast<uint32_t>(std::max(std::abs(a.x - b.x), std::abs(a.y - b.y)));
        }

    public:
        /**
         * @brief Evaluates decay and dispatches contractors if thresholds are breached.
         */
        void process_maintenance_needs(ECSWorld& world, EventPublisher& publisher) {
            auto bld_view = world.registry.view<StructuralQualityComponent, MaintenanceStateComponent, BuildingIdentityComponent>();
            auto contractor_view = world.registry.view<Labour::ContractorComponent>();

            for (auto raw_bld : bld_view) {
                auto& quality = bld_view.get<StructuralQualityComponent>(raw_bld);
                auto& state = bld_view.get<MaintenanceStateComponent>(raw_bld);
                const auto& identity = bld_view.get<BuildingIdentityComponent>(raw_bld);

                // 1. Apply Continuous Degradation
                // dQ/dt = -(0.002 + 0.005 * (100 - Q) / 100)
                float decay = 0.002f + 0.005f * ((100.0f - quality.current_quality) / 100.0f);
                quality.current_quality -= decay;
                if (quality.current_quality < 0.0f) quality.current_quality = 0.0f;

                // 2. Threshold Check
                if (quality.current_quality < 60.0f && !state.has_active_work_order) {

                    MaintenanceUrgency urgency = (quality.current_quality < 30.0f) ? MaintenanceUrgency::HIGH : MaintenanceUrgency::NORMAL;

                    EntityID best_contractor{0};
                    float best_score = -1.0f;

                    // 3. Search for the optimal contractor
                    for (auto raw_cont : contractor_view) {
                        auto& contractor = contractor_view.get<Labour::ContractorComponent>(raw_cont);

                        if (!contractor.is_available) continue;

                        uint32_t dist = get_chebyshev_distance(identity.address, contractor.location);
                        float proximity_score = 1.0f / (1.0f + static_cast<float>(dist) * 0.1f);
                        float skill_score = static_cast<float>(contractor.skill_level) / 20.0f; // Normalize 0-1

                        // Formula: proximity * 0.4 + skill * 0.6
                        float score = (proximity_score * 0.4f) + (skill_score * 0.6f);

                        if (score > best_score) {
                            best_score = score;
                            best_contractor = EntityID{static_cast<uint32_t>(raw_cont)};
                        }
                    }

                    // 4. Dispatch the Order
                    if (best_contractor.raw_id != 0) {
                        state.has_active_work_order = true;
                        state.assigned_contractor = best_contractor;

                        // Mark contractor as busy
                        world.registry.get<Labour::ContractorComponent>(static_cast<entt::entity>(best_contractor.raw_id)).is_available = false;

                        EntityID bld_id{static_cast<uint32_t>(raw_bld)};
                        publisher.publish(WorkOrderIssuedEvent{bld_id, best_contractor, urgency});
                    }
                }
            }
        }

        /**
         * @brief Called when a contractor finishes their job at the site.
         */
        void complete_maintenance(ECSWorld& world, EventPublisher& publisher, EntityID bld_id) {
            entt::entity raw_bld = static_cast<entt::entity>(bld_id.raw_id);
            if (!world.registry.valid(raw_bld)) return;

            auto& quality = world.registry.get<StructuralQualityComponent>(raw_bld);
            auto& state = world.registry.get<MaintenanceStateComponent>(raw_bld);

            if (!state.has_active_work_order) return;

            entt::entity raw_cont = static_cast<entt::entity>(state.assigned_contractor.raw_id);
            float skill = 10.0f; // Default fallback

            if (world.registry.valid(raw_cont) && world.registry.all_of<Labour::ContractorComponent>(raw_cont)) {
                auto& contractor = world.registry.get<Labour::ContractorComponent>(raw_cont);
                skill = static_cast<float>(contractor.skill_level);
                contractor.is_available = true; // Free them up for the next job!
            }

            // Formula: Q_restored = 90.0 + contractor_skill_level * 0.5
            quality.current_quality = 90.0f + (skill * 0.5f);
            if (quality.current_quality > 100.0f) quality.current_quality = 100.0f;

            state.has_active_work_order = false;
            state.assigned_contractor = EntityID{0};

            publisher.publish(MaintenanceCompletedEvent{bld_id, state.assigned_contractor, quality.current_quality});
        }
    };
}