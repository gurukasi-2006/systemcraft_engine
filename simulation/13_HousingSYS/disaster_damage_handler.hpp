#pragma once

#include <cstdint>
#include <algorithm>
#include <cmath>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/03_Event_Bus/subscriber_registry.hpp"
#include "../../../core/04_Types/tile_coord.hpp"
#include "../../../core/04_Types/time_constants.hpp"

// Required Housing Components from Phase 4
#include "../../housing/12_HousingComponent/structural_quality_component.hpp"
#include "../../housing/12_HousingComponent/building_identity_component.hpp"
#include "../../housing/12_HousingComponent/demolition_candidate_component.hpp"
#include "../../housing/12_HousingComponent/energy_efficiency_component.hpp"
#include "../../housing/12_HousingComponent/floor_area_density_component.hpp"

/**
 * @file disaster_damage_handler.hpp
 * @brief Subsystem 166: Applies physical physics formulas to buildings during disasters and triggers emergencies.
 */

namespace Disasters {

    enum class DisasterType : uint8_t {
        Fire = 0,
        Flood,
        Earthquake,
        Sinkhole,  // NEW: Punishes high-density FAR buildings
        Heatwave   // NEW: Punishes poor insulation
    };

    // --- Inbound Events ---
    struct DisasterOccurredEvent {
        DisasterType type;
        TileCoord epicenter;
        uint32_t radius;
        float intensity;       // Used by Fire (0.0 - 1.0)
        uint32_t days;         // Used by Flood
        float magnitude;       // Used by Earthquake (1.0 - 10.0)
        float severity;        // Used by Heatwave (1.0 - 10.0)
    };

    // --- Outbound Emergency Events ---
    struct EmergencyHousingSurgeEvent {
        uint32_t displaced_citizens_est;
    };
    struct TemporaryDisplacementEvent {
        uint32_t flooded_buildings;
    };
    struct MassEvacuationEvent {
        uint32_t collapsed_buildings;
    };

    /**
     * @class DisasterDamageHandler
     * @brief Listens for disasters and mathematically applies structural damage.
     */
    class DisasterDamageHandler {
    private:
        uint32_t get_chebyshev_distance(TileCoord a, TileCoord b) const {
            return static_cast<uint32_t>(std::max(std::abs(static_cast<int32_t>(a.x) - static_cast<int32_t>(b.x)),
                                                  std::abs(static_cast<int32_t>(a.y) - static_cast<int32_t>(b.y))));
        }

    public:
        DisasterDamageHandler(SubscriberRegistry& registry, ECSWorld& world, EventPublisher& publisher, uint64_t* current_tick_ptr) {

            registry.subscribe<DisasterOccurredEvent>([&world, &publisher, current_tick_ptr, this](const DisasterOccurredEvent& ev) {
                auto view = world.registry.view<Housing::BuildingIdentityComponent, Housing::StructuralQualityComponent>();

                uint32_t destroyed_count = 0;
                uint32_t affected_count = 0;

                for (auto raw : view) {
                    const auto& id = view.get<Housing::BuildingIdentityComponent>(raw);
                    auto& quality = view.get<Housing::StructuralQualityComponent>(raw);

                    // Range Check
                    uint32_t dist = this->get_chebyshev_distance(ev.epicenter, id.address);
                    if (dist > ev.radius) continue;

                    affected_count++;
                    float damage = 0.0f;

                    // --- Damage Formulas ---
                    switch (ev.type) {
                        case DisasterType::Fire:
                            damage = ev.intensity * 60.0f;
                            break;

                        case DisasterType::Flood:
                            damage = 20.0f + (static_cast<float>(ev.days) * 5.0f);
                            break;

                        case DisasterType::Earthquake: {
                            // Age-amplified damage (1.0 + age_years * 0.02)
                            float age_years = 0.0f;
                            if (*current_tick_ptr > id.construction_tick) {
                                age_years = static_cast<float>(*current_tick_ptr - id.construction_tick) / static_cast<float>(TimeConstants::TICKS_PER_YEAR);
                            }
                            float age_factor = 1.0f + (age_years * 0.02f);
                            damage = ev.magnitude * 15.0f * age_factor;
                            break;
                        }

                        case DisasterType::Sinkhole: {
                            // Instant kill for heavy high-density buildings
                            if (world.registry.all_of<Housing::FloorAreaDensityComponent>(raw)) {
                                if (world.registry.get<Housing::FloorAreaDensityComponent>(raw).floor_area_ratio > 3.0f) {
                                    damage = 999.0f; // Total structural failure
                                } else {
                                    damage = 30.0f; // Light buildings survive sinking better
                                }
                            }
                            break;
                        }

                        case DisasterType::Heatwave: {
                            // Thermal expansion cracks rely on insulation (0.0 to 1.0)
                            float insulation = 0.0f;
                            if (world.registry.all_of<Housing::EnergyEfficiencyComponent>(raw)) {
                                insulation = world.registry.get<Housing::EnergyEfficiencyComponent>(raw).insulation_rating;
                            }
                            damage = ev.severity * 10.0f * (1.0f - insulation);
                            break;
                        }
                    }

                    // Apply Damage
                    quality.current_quality -= damage;

                    // Catastrophic Collapse Check
                    if (quality.current_quality <= 0.0f) {
                        quality.current_quality = 0.0f;
                        destroyed_count++;

                        // Mark for emergency demolition
                        world.registry.emplace_or_replace<Housing::DemolitionCandidateComponent>(raw,
                            Housing::DemolitionCandidateComponent{Housing::DemolitionReason::DisasterDamage}
                        );
                    }
                }

                // --- Dispatch Macro Emergency Events ---
                if (ev.type == DisasterType::Fire && destroyed_count > 0) {
                    publisher.publish(EmergencyHousingSurgeEvent{destroyed_count * 4}); // Rough citizen estimate
                }
                else if (ev.type == DisasterType::Flood && affected_count > 0) {
                    publisher.publish(TemporaryDisplacementEvent{affected_count});
                }
                else if (ev.type == DisasterType::Earthquake && destroyed_count > 0) {
                    publisher.publish(MassEvacuationEvent{destroyed_count});
                }
            });
        }
    };
}