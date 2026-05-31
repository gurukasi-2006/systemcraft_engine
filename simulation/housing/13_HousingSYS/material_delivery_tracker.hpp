#pragma once

#include <cstdint>
#include <algorithm>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/03_Event_Bus/subscriber_registry.hpp"
#include "../../../core/04_Types/time_constants.hpp"

#include "../12_HousingComponent/construction_progress_component.hpp"
#include "../12_HousingComponent/demolition_candidate_component.hpp" // To tear down 6-month stalls

/**
 * @file material_delivery_tracker.hpp
 * @brief Subsystem 157: Monitors logistics deliveries and detects "Zombie Construction" supply chain stalls.
 */

namespace Housing {

    /**
     * @enum ResourceType
     * @brief The physical materials required to erect a building.
     */
    enum class ResourceType : uint8_t {
        Cement = 0,
        Steel,
        Timber,
        ConsumerGoods
    };

    // --- Logistics Events ---

    struct MaterialDeliveredEvent {
        EntityID building_id;
        ResourceType resource_type;
        float quantity;
        uint64_t delivery_tick;
    };

    struct PhaseCompleteEvent {
        EntityID building_id;
        ConstructionPhase new_phase;
    };

    struct ConstructionStalledEvent {
        EntityID building_id;
        ResourceType missing_material;
    };

    /**
     * @struct MaterialDeliveryStateComponent
     * @brief ECS Component attached to active construction sites to track logistics health.
     */
    struct MaterialDeliveryStateComponent {
        uint64_t last_delivery_tick{0};
        bool is_stalled{false};
        ConstructionPhase last_known_phase{ConstructionPhase::Planned};
    };

    /**
     * @class MaterialDeliveryTracker
     * @brief Hooks into the Event Bus to process deliveries and scan for stalled sites.
     */
    class MaterialDeliveryTracker {
    public:
        /**
         * @brief Subscribes to the logistics grid to process incoming material drops.
         */
        MaterialDeliveryTracker(SubscriberRegistry& registry, ECSWorld& world, EventPublisher& publisher) {
            registry.subscribe<MaterialDeliveredEvent>([&world, &publisher](const MaterialDeliveredEvent& ev) {
                entt::entity raw = static_cast<entt::entity>(ev.building_id.raw_id);

                if (world.registry.valid(raw) &&
                    world.registry.all_of<ConstructionProgressComponent, MaterialDeliveryStateComponent>(raw)) {

                    auto& progress = world.registry.get<ConstructionProgressComponent>(raw);
                    auto& state = world.registry.get<MaterialDeliveryStateComponent>(raw);

                    // 1. Log the delivery
                    state.last_delivery_tick = ev.delivery_tick;
                    state.is_stalled = false; // Delivery un-stalls the site

                    switch (ev.resource_type) {
                        case ResourceType::Cement:        progress.cement_delivered += ev.quantity; break;
                        case ResourceType::Steel:         progress.steel_delivered += ev.quantity; break;
                        case ResourceType::Timber:        progress.timber_delivered += ev.quantity; break;
                        case ResourceType::ConsumerGoods: progress.consumer_goods_delivered += ev.quantity; break;
                    }

                    // 2. Check if a phase just completed (We simulate a micro labor tick to update the phase cap)
                    progress.advance_construction(0.01f);

                    if (progress.phase != state.last_known_phase) {
                        publisher.publish(PhaseCompleteEvent{ev.building_id, progress.phase});
                        state.last_known_phase = progress.phase;
                    }
                }
            });
        }

        /**
         * @brief Scans all active construction sites to detect supply chain failures.
         */
        void check_stalls(ECSWorld& world, EventPublisher& publisher, uint64_t current_tick) {
            uint64_t ticks_per_month = TimeConstants::TICKS_PER_YEAR / 12;
            auto view = world.registry.view<ConstructionProgressComponent, MaterialDeliveryStateComponent>();

            for (auto raw : view) {
                const auto& progress = view.get<ConstructionProgressComponent>(raw);
                auto& state = view.get<MaterialDeliveryStateComponent>(raw);

                // Ignore finished buildings
                if (progress.phase == ConstructionPhase::Complete) continue;

                uint64_t ticks_since_delivery = current_tick - state.last_delivery_tick;

                // --- 6-Month Stall: Developer Cancels Project ---
                if (ticks_since_delivery > (ticks_per_month * 6)) {
                    // Turn it into a derelict demolition candidate
                    world.registry.emplace_or_replace<DemolitionCandidateComponent>(raw,
                        DemolitionCandidateComponent{DemolitionReason::Derelict}
                    );
                    continue;
                }

                // --- 2-Month Stall: Trigger Stalled Event ---
                if (ticks_since_delivery > (ticks_per_month * 2) && !state.is_stalled) {
                    state.is_stalled = true;

                    ResourceType missing = ResourceType::Cement;
                    if (progress.cement_delivered < progress.cement_required) missing = ResourceType::Cement;
                    else if (progress.steel_delivered < progress.steel_required) missing = ResourceType::Steel;
                    else if (progress.timber_delivered < progress.timber_required) missing = ResourceType::Timber;
                    else missing = ResourceType::ConsumerGoods;

                    EntityID bld_id{static_cast<uint32_t>(raw)};
                    publisher.publish(ConstructionStalledEvent{bld_id, missing});
                }
            }
        }
    };
}