#pragma once

#include <entt/entt.hpp>
#include <array>
#include <algorithm>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/tile_coord.hpp"

#include "need_definitions.hpp" // Subsystem 115
#include "../09_CitizenSpawner/citizen_spawner.hpp" // For PositionComponent

/**
 * @file unmet_need_event_emitter.hpp
 * @brief Subsystem 124: Detects critical deprivations and fires localized events for UI heatmaps and crises.
 */

struct NeedUnmetEvent {
    EntityID citizen;
    Population::NeedType need_type;
    float severity; // 0.0 (Just crossed threshold) to 1.0 (Absolute zero satisfaction)
    TileCoord location; // The crucial data point for Spatial Heatmaps!
};

class UnmetNeedEventEmitter {
private:
    // Mapped exactly to the 13 NeedType enums from Subsystem 115
    static constexpr std::array<float, static_cast<size_t>(Population::NeedType::COUNT)> CRITICAL_THRESHOLDS = {
        40.0f,  // 0: Food
        35.0f,  // 1: Water
        30.0f,  // 2: Shelter
        20.0f,  // 3: Healthcare
        25.0f,  // 4: Safety
        35.0f,  // 5: Employment
        30.0f,  // 6: Transport (Extrapolated Baseline)
        30.0f,  // 7: Education
        30.0f,  // 8: Leisure
        30.0f,  // 9: Community
        30.0f,  // 10: Environment
        30.0f,  // 11: ConsumerGoods
        30.0f   // 12: Connectivity
    };

public:
    /**
     * @brief Scans all citizens and emits a localized event if any need drops below its critical threshold.
     * @param world The ECS master world.
     * @param publisher The global event bus.
     */
    void update(ECSWorld& world, EventPublisher& publisher) {
        auto view = world.registry.view<PositionComponent, Population::NeedsComponent>();

        for (auto raw_id : view) {
            const auto& pos = view.get<PositionComponent>(raw_id);
            const auto& needs = view.get<Population::NeedsComponent>(raw_id);
            EntityID citizen{static_cast<uint32_t>(raw_id)};

            // Scan all 13 needs in an ultra-fast loop
            for (size_t i = 0; i < static_cast<size_t>(Population::NeedType::COUNT); ++i) {
                float current_sat = needs.satisfaction_levels[i];
                float threshold = CRITICAL_THRESHOLDS[i];

                if (current_sat < threshold) {
                    // Severity Formula: (Threshold - Current) / Threshold
                    // Example: Threshold is 40. Current is 20. (40 - 20) / 40 = 0.5 (50% Severity)
                    float severity = (threshold - current_sat) / threshold;

                    // Clamp just to be absolutely mathematically safe against weird data states
                    severity = std::clamp(severity, 0.0f, 1.0f);

                    // Fire the macro-event for the UI Heatmaps and the Crisis Engine!
                    publisher.publish(NeedUnmetEvent{
                        citizen,
                        static_cast<Population::NeedType>(i),
                        severity,
                        pos.coord
                    });
                }
            }
        }
    }
};