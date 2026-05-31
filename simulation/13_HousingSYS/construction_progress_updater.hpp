#pragma once

#include <cstdint>
#include <algorithm>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"

#include "../12_HousingComponent/construction_progress_component.hpp"
#include "material_delivery_tracker.hpp" // For PhaseCompleteEvent

/**
 * @file construction_progress_updater.hpp
 * @brief Subsystem 158: Ticks active construction sites based on labor efficiency and material availability.
 */

namespace Housing {

    /**
     * @struct ConstructionLaborComponent
     * @brief Supplemental data for construction sites detailing workforce and wage policies.
     */
    struct ConstructionLaborComponent {
        uint32_t workers_required{10};
        bool developer_pays_overtime{false};
    };

    /**
     * @class ConstructionProgressUpdater
     * @brief System that applies physical labor to building sites every tick.
     */
    class ConstructionProgressUpdater {
    public:
        /**
         * @brief Updates the completion percentage of all active construction projects.
         */
        void update(ECSWorld& world, EventPublisher& publisher) {
            auto view = world.registry.view<ConstructionProgressComponent, ConstructionLaborComponent>();

            for (auto raw : view) {
                auto& progress = view.get<ConstructionProgressComponent>(raw);
                const auto& labor = view.get<ConstructionLaborComponent>(raw);

                if (progress.phase == ConstructionPhase::Complete) continue;

                // --- 1. Calculate Worker Efficiency ---
                float worker_ratio = static_cast<float>(progress.workers_assigned) /
                                     static_cast<float>(std::max(1u, labor.workers_required));

                float worker_eff = std::clamp(worker_ratio, 0.1f, 1.5f);

                // Overtime modifier: 30% speed boost
                if (labor.developer_pays_overtime) {
                    worker_eff *= 1.3f;
                }

                // --- 2. Calculate Material Factor ---
                float mat_factor = 1.0f;
                switch (progress.phase) {
                    case ConstructionPhase::Planned:
                        if (progress.cement_required > 0.0f) {
                            mat_factor = std::min(1.0f, progress.cement_delivered / progress.cement_required);
                        }
                        break;
                    case ConstructionPhase::Foundation:
                        {
                            float req = progress.steel_required + progress.timber_required;
                            if (req > 0.0f) {
                                float del = progress.steel_delivered + progress.timber_delivered;
                                mat_factor = std::min(1.0f, del / req);
                            }
                        }
                        break;
                    case ConstructionPhase::Framing:
                        mat_factor = 1.0f; // Framing (40->80) is purely labor-driven once steel/timber are placed
                        break;
                    case ConstructionPhase::Finishing:
                        if (progress.consumer_goods_required > 0.0f) {
                            mat_factor = std::min(1.0f, progress.consumer_goods_delivered / progress.consumer_goods_required);
                        }
                        break;
                }

                // --- 3. Apply Progress ---
                // Base rate: 1% per tick at full staffing
                float progress_gain_per_tick = 0.01f * worker_eff * mat_factor;

                auto old_phase = progress.phase;

                // We use the subsystem 147 advance_construction to respect hard material caps
                progress.advance_construction(progress_gain_per_tick);

                // --- 4. Fire Phase Complete Events ---
                if (progress.phase != old_phase) {
                    EntityID bld_id{static_cast<uint32_t>(raw)};
                    publisher.publish(PhaseCompleteEvent{bld_id, progress.phase});
                }
            }
        }
    };
}