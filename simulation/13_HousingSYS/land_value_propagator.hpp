#pragma once

#include <cstdint>
#include <unordered_map>
#include <cmath>
#include <algorithm>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/03_Event_Bus/subscriber_registry.hpp"
#include "../../../core/04_Types/tile_coord.hpp"

// Required Components
#include "../12_HousingComponent/building_identity_component.hpp"
#include "../12_HousingComponent/structural_quality_component.hpp"
#include "../12_HousingComponent/land_value_influence_component.hpp"
#include "../12_HousingComponent/construction_progress_component.hpp" // For PhaseCompleteEvent
#include "demolition_executor.hpp" // For BuildingDemolishedEvent

/**
 * @file land_value_propagator.hpp
 * @brief Subsystem 164: Manages the continuous spatial field of land values and gentrification auras.
 */

namespace Housing {

    /**
     * @class LandValuePropagator
     * @brief Maintains the 2D land value map and processes delta updates and monthly drift sweeps.
     */
    class LandValuePropagator {
    private:
        std::unordered_map<uint64_t, float> land_value_map_;
        float base_terrain_value_{50.0f};

        uint64_t get_tile_key(TileCoord coord) const {
            return (static_cast<uint64_t>(coord.x) << 32) | static_cast<uint64_t>(coord.y);
        }

        uint32_t get_influence_radius(BuildingType type) const {
            switch (type) {
                case BuildingType::Shack:      return 3;
                case BuildingType::House:      return 4;
                case BuildingType::LowRiseApt: return 6;
                case BuildingType::Tower:      return 9;
                default:                       return 5;
            }
        }

        /**
         * @brief Applies a building's aura to the map (Positive for additions, Negative for removals)
         */
        void apply_building_influence(const BuildingIdentityComponent& id, const StructuralQualityComponent& qual, float multiplier) {
            LandValueInfluenceComponent math_helper;
            uint32_t radius = get_influence_radius(id.type);
            float base_emit = math_helper.get_base_emit(qual.current_quality, id.type) * multiplier;

            // Apply to all tiles within the Chebyshev radius
            int32_t r = static_cast<int32_t>(radius);
            for (int32_t dx = -r; dx <= r; ++dx) {
                for (int32_t dy = -r; dy <= r; ++dy) {
                    int32_t target_x = static_cast<int32_t>(id.address.x) + dx;
                    int32_t target_y = static_cast<int32_t>(id.address.y) + dy;

                    if (target_x < 0 || target_y < 0) continue; // Grid bounds safety

                    TileCoord target{target_x, target_y};
                    uint32_t dist = math_helper.get_chebyshev_distance(id.address, target);

                    if (dist <= radius) {
                        float delta = base_emit * std::exp(-0.3f * static_cast<float>(dist));
                        land_value_map_[get_tile_key(target)] += delta;
                    }
                }
            }
        }

    public:
        /**
         * @brief Hooks into the event bus to listen for construction completions and demolitions.
         */
        LandValuePropagator(SubscriberRegistry& registry, ECSWorld& world) {
            // New Building Addition
            registry.subscribe<PhaseCompleteEvent>([this, &world](const PhaseCompleteEvent& ev) {
                if (ev.new_phase == ConstructionPhase::Complete) {
                    entt::entity raw = static_cast<entt::entity>(ev.building_id.raw_id);
                    if (world.registry.all_of<BuildingIdentityComponent, StructuralQualityComponent>(raw)) {
                        const auto& id = world.registry.get<BuildingIdentityComponent>(raw);
                        const auto& qual = world.registry.get<StructuralQualityComponent>(raw);
                        this->apply_building_influence(id, qual, 1.0f); // Add aura
                    }
                }
            });

            // Building Demolition Removal
            registry.subscribe<BuildingDemolishedEvent>([this, &world](const BuildingDemolishedEvent& ev) {
                entt::entity raw = static_cast<entt::entity>(ev.building_id.raw_id);
                // Demolition executor leaves entity alive until the Done phase, so we can still read its stats!
                if (world.registry.all_of<BuildingIdentityComponent, StructuralQualityComponent>(raw)) {
                    const auto& id = world.registry.get<BuildingIdentityComponent>(raw);
                    const auto& qual = world.registry.get<StructuralQualityComponent>(raw);
                    this->apply_building_influence(id, qual, -1.0f); // Subtract aura
                }
            });
        }

        /**
         * @brief Re-accumulates the entire city's land value from scratch to fix floating-point drift.
         */
        void run_monthly_sweep(ECSWorld& world) {
            land_value_map_.clear();

            auto view = world.registry.view<BuildingIdentityComponent, StructuralQualityComponent>();
            for (auto raw : view) {
                // Only count finished buildings
                if (world.registry.all_of<ConstructionProgressComponent>(raw)) {
                    if (world.registry.get<ConstructionProgressComponent>(raw).phase != ConstructionPhase::Complete) {
                        continue;
                    }
                }
                const auto& id = view.get<BuildingIdentityComponent>(raw);
                const auto& qual = view.get<StructuralQualityComponent>(raw);
                apply_building_influence(id, qual, 1.0f);
            }
        }

        /**
         * @brief Retrieves the final calculated value for a specific tile.
         */
        float get_land_value(TileCoord coord) const {
            float val = base_terrain_value_;
            auto it = land_value_map_.find(get_tile_key(coord));
            if (it != land_value_map_.end()) {
                val += it->second;
            }
            return val;
        }
    };
}