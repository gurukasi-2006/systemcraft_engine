#pragma once

#include <array>
#include <cstdint>

#include "../../../core/03_Event_Bus/event_publisher.hpp"

/**
 * @file population_milestone_detector.hpp
 * @brief Subsystem 140: Fires one-shot events when population crosses specific thresholds.
 */

namespace Demographics {

    /**
     * @struct MilestoneReachedEvent
     * @brief Fired exactly once per threshold to unlock infrastructure tiers and achievements.
     */
    struct MilestoneReachedEvent {
        uint32_t threshold;
        uint32_t current_pop;
    };

    class PopulationMilestoneDetector {
    private:
        // The fixed progression thresholds from the design document
        static constexpr std::array<uint32_t, 7> MILESTONES = {
            10000, 50000, 100000, 500000, 1000000, 5000000, 10000000
        };

        // Tracks which milestones have already been triggered
        std::array<bool, 7> milestone_fired_{false};

    public:
        /**
         * @brief Checks the current population against the progression milestones.
         * @param publisher The global event bus.
         * @param current_pop The live total population (provided by Subsystem 128: Population Counter).
         */
        void update(EventPublisher& publisher, uint32_t current_pop) {
            for (size_t i = 0; i < MILESTONES.size(); ++i) {
                // If the milestone hasn't fired yet, and we reached or exceeded it
                if (!milestone_fired_[i] && current_pop >= MILESTONES[i]) {

                    milestone_fired_[i] = true; // Mark as one-shot complete

                    publisher.publish(MilestoneReachedEvent{
                        MILESTONES[i],
                        current_pop
                    });
                }
            }
        }

        /**
         * @brief Resets the detector (Useful for testing or loading a new save file).
         */
        void reset() {
            milestone_fired_.fill(false);
        }
    };
}