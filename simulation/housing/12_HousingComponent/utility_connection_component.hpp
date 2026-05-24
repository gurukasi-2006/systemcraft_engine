#pragma once

#include <cstdint>
#include <string_view>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/entity_id.hpp"
#include "../../../core/04_Types/time_constants.hpp"

#include "structural_quality_component.hpp"
#include "occupancy_component.hpp"

// Fallback declarations for Phase 8 components if needed
#include "../../citizens/08_Citizencomponent/health_component.hpp"

#ifndef NEEDS_COMPONENT_HPP
#define NEEDS_COMPONENT_HPP
namespace Population {
    struct NeedsComponent {
        float food{100.0f};
        float leisure{100.0f};
        float safety{100.0f};
    };
}
#endif

/**
 * @file utility_connection_component.hpp
 * @brief Subsystem 146: Tracks water, electricity, and sewage, applying severe localized penalties when disconnected.
 */

namespace Housing {

    // Bitmask Definitions
    constexpr uint8_t WATER_BIT  = 0x01; // 0000 0001
    constexpr uint8_t ELEC_BIT   = 0x02; // 0000 0010
    constexpr uint8_t SEWAGE_BIT = 0x04; // 0000 0100

    /**
     * @struct DiseaseOutbreakRiskEvent
     * @brief Fired daily when poor sanitation creates a localized epidemic risk.
     */
    struct DiseaseOutbreakRiskEvent {
        EntityID building_id;
        EntityID citizen_id;
        std::string_view disease_name;
        float probability; // 0.0 to 1.0 chance of contracting it today
    };

    /**
     * @struct UtilityConnectionComponent
     * @brief A bitfield tracking infrastructure connectivity.
     */
    struct UtilityConnectionComponent {
        uint8_t utility_flags{0b00000000}; // Defaults to all disconnected

        bool has_water() const       { return (utility_flags & WATER_BIT) != 0; }
        bool has_electricity() const { return (utility_flags & ELEC_BIT) != 0; }
        bool has_sewage() const      { return (utility_flags & SEWAGE_BIT) != 0; }

        void set_water(bool state)       { if (state) utility_flags |= WATER_BIT; else utility_flags &= ~WATER_BIT; }
        void set_electricity(bool state) { if (state) utility_flags |= ELEC_BIT; else utility_flags &= ~ELEC_BIT; }
        void set_sewage(bool state)      { if (state) utility_flags |= SEWAGE_BIT; else utility_flags &= ~SEWAGE_BIT; }

        /**
         * @brief Assesses utilities and applies penalties to the building and its occupants.
         */
        void process_utilities(ECSWorld& world, EventPublisher& publisher, EntityID self_id, uint64_t current_tick) {
            entt::entity raw_self = static_cast<entt::entity>(self_id.raw_id);
            if (!world.registry.valid(raw_self)) return;

            bool water = has_water();
            bool elec = has_electricity();
            bool sewage = has_sewage();

            // If everything is perfectly connected, do nothing!
            if (water && elec && sewage) return;

            // --- 1. Calculate Per-Tick Penalties ---
            float quality_decay = 0.0f;
            float food_penalty = 0.0f;
            float leisure_penalty = 0.0f;
            float safety_penalty = 0.0f;
            float health_penalty = 0.0f;

            if (!water) {
                quality_decay += 0.003f;
                food_penalty += 0.02f;
            }
            if (!elec) {
                quality_decay += 0.001f;
                leisure_penalty += 0.015f;
                safety_penalty += 0.010f;
            }
            if (!sewage) {
                quality_decay += 0.005f;
                health_penalty += 0.005f;
            }

            // --- 2. Apply Structural Decay ---
            if (world.registry.all_of<StructuralQualityComponent>(raw_self)) {
                auto& structure = world.registry.get<StructuralQualityComponent>(raw_self);
                structure.current_quality -= quality_decay;
                if (structure.current_quality < 0.0f) structure.current_quality = 0.0f;
            }

            // --- 3. Apply Occupant Needs & Health Penalties ---
            if (world.registry.all_of<OccupancyComponent>(raw_self)) {
                const auto& occupancy = world.registry.get<OccupancyComponent>(raw_self);

                for (EntityID cit_id : occupancy.occupant_list) {
                    entt::entity raw_cit = static_cast<entt::entity>(cit_id.raw_id);
                    if (!world.registry.valid(raw_cit)) continue;

                    // Apply Needs Decay
                    if (world.registry.all_of<Population::NeedsComponent>(raw_cit)) {
                        auto& needs = world.registry.get<Population::NeedsComponent>(raw_cit);
                        needs.food -= food_penalty;
                        needs.leisure -= leisure_penalty;
                        needs.safety -= safety_penalty;

                        if (needs.food < 0.0f) needs.food = 0.0f;
                        if (needs.leisure < 0.0f) needs.leisure = 0.0f;
                        if (needs.safety < 0.0f) needs.safety = 0.0f;
                    }

                    // Apply Base Health Decay
                    if (world.registry.all_of<Population::HealthComponent>(raw_cit)) {
                        auto& health = world.registry.get<Population::HealthComponent>(raw_cit);
                        if constexpr (requires { health.physical_health; }) {
                            health.physical_health -= health_penalty;
                            if (health.physical_health < 0.0f) health.physical_health = 0.0f;
                        } else if constexpr (requires { health.physical_health; }) {
                            health.physical_health -= health_penalty;
                            if (health.physical_health < 0.0f) health.physical_health = 0.0f;
                        }
                    }

                    // --- 4. Spatial Epidemic Triggers (Daily) ---
                    if (current_tick > 0 && current_tick % TimeConstants::TICKS_PER_DAY == 0) {
                        // 3x Multiplier if everything is broken
                        float disease_multiplier = (!water && !elec && !sewage) ? 3.0f : 1.0f;

                        if (!water) {
                            publisher.publish(DiseaseOutbreakRiskEvent{
                                self_id, cit_id, "Cholera", 0.001f * disease_multiplier
                            });
                        }
                        if (!sewage) {
                            publisher.publish(DiseaseOutbreakRiskEvent{
                                self_id, cit_id, "Typhoid", 0.002f * disease_multiplier
                            });
                        }
                    }
                }
            }
        }
    };
}