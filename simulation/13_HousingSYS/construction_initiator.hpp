#pragma once

#include <cstdint>
#include <vector>
#include <utility>
#include <algorithm>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/04_Types/tile_coord.hpp"
#include "../../../core/04_Types/entity_id.hpp"

// Required components to initialize a new building site
#include "../12_HousingComponent/building_identity_component.hpp"
#include "../12_HousingComponent/construction_progress_component.hpp"

/**
 * @file construction_initiator.hpp
 * @brief Subsystem 156: Developer AI that scores plots and initiates construction when demand spikes.
 */

namespace Housing {

    /**
     * @struct PlotEvaluationContext
     * @brief Normalized metrics (0.0 to 1.0) provided by the grid for AI scoring.
     */
    struct PlotEvaluationContext {
        bool has_road{false};
        bool has_water_pipe{false};
        bool has_electric_grid{false};
        bool has_sewage{false};

        float land_value_score{0.0f};           // 30% Weight
        float demand_proximity_score{0.0f};     // 25% Weight
        float zoning_compatibility_score{0.0f}; // 15% Weight
        float construction_cost_score{0.0f};    // 10% Weight (Higher is better/cheaper)
    };

    /**
     * @class ConstructionInitiator
     * @brief Evaluates land and spawns new building entities in the 'Planned' phase.
     */
    class ConstructionInitiator {
    public:
        /**
         * @brief Calculates the infrastructure access score based on utility availability.
         */
        float calculate_infrastructure_score(const PlotEvaluationContext& ctx) const {
            float score = 0.0f;
            if (ctx.has_road)          score += 0.40f;
            if (ctx.has_water_pipe)    score += 0.30f;
            if (ctx.has_electric_grid) score += 0.20f;
            if (ctx.has_sewage)        score += 0.10f;
            return score;
        }

        /**
         * @brief Applies the multi-criterion weighting formula to determine overall plot viability.
         */
        float calculate_plot_score(const PlotEvaluationContext& ctx) const {
            float infra_score = calculate_infrastructure_score(ctx);

            return (0.30f * ctx.land_value_score) +
                   (0.25f * ctx.demand_proximity_score) +
                   (0.20f * infra_score) +
                   (0.15f * ctx.zoning_compatibility_score) +
                   (0.10f * ctx.construction_cost_score);
        }

        /**
         * @brief Selects the absolute best plot from a list of surveyed candidates.
         */
        TileCoord select_best_plot(const std::vector<std::pair<TileCoord, PlotEvaluationContext>>& candidates) const {
            TileCoord best_plot{0, 0};
            float best_score = -1.0f;

            for (const auto& [coord, ctx] : candidates) {
                float score = calculate_plot_score(ctx);

                // Real-world planning rule: Plots without roads are virtually ineligible
                if (!ctx.has_road) {
                    score *= 0.1f; // Severely penalize landlocked plots
                }

                if (score > best_score) {
                    best_score = score;
                    best_plot = coord;
                }
            }
            return best_plot;
        }

        /**
         * @brief Spawns the ECS entity, officially beginning the construction pipeline.
         */
        EntityID initiate_construction(ECSWorld& world, TileCoord coord, BuildingType type, float floor_area_m2, uint32_t num_floors, uint64_t current_tick) const {
            EntityID bld_id = world.entity_manager.createEntity();
            entt::entity raw = static_cast<entt::entity>(bld_id.raw_id);

            // 1. Establish Immutable Identity
            world.registry.emplace<BuildingIdentityComponent>(raw,
                BuildingIdentityComponent{bld_id, current_tick, coord, type}
            );

            // 2. Queue into the Supply Chain Pipeline (0% Planned Phase)
            auto& progress = world.registry.emplace<ConstructionProgressComponent>(raw);
            progress.phase = ConstructionPhase::Planned;
            progress.progress_pct = 0.0f;
            progress.initialize_requirements(type, floor_area_m2, num_floors);

            return bld_id;
        }
    };
}