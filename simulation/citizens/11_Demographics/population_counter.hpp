#pragma once

#include <unordered_map>
#include <cstdint>

#include "../../../core/03_Event_Bus/subscriber_registry.hpp"
#include "../../../core/04_Types/entity_id.hpp"

// Include the events we are subscribing to
#include "../09_CitizenSpawner/migration_executor.hpp" // For CitizenMigratedEvent
#include "../09_CitizenSpawner/death_handler.hpp"      // For CitizenDiedEvent

/**
 * @file population_counter.hpp
 * @brief Subsystem 128: Maintains live total and per-region citizen counts via O(1) Event Bus subscriptions.
 */

namespace Population {
    using RegionID = uint32_t;
}

// Ensure CitizenSpawnedEvent exists (Fallback definition if it wasn't explicitly defined in Phase 9)
#ifndef CITIZEN_SPAWNED_EVENT_HPP
#define CITIZEN_SPAWNED_EVENT_HPP
struct CitizenSpawnedEvent {
    EntityID citizen;
    Population::RegionID region_id;
};
#endif

namespace Demographics {

    class PopulationCounter {
    private:
        uint32_t total_count_{0};
        std::unordered_map<Population::RegionID, uint32_t> region_counts_;

        // O(1) tracker to remember where a citizen lived in case CitizenDiedEvent lacks spatial data
        std::unordered_map<uint32_t, Population::RegionID> citizen_tracker_;

    public:
        /**
         * @brief Binds the population counter to the central Event Bus.
         * @param registry The event subscriber registry.
         */
        PopulationCounter(SubscriberRegistry& registry) {

            // --- 1. Handle Births / Spawning ---
            registry.subscribe<CitizenSpawnedEvent>([this](const CitizenSpawnedEvent& ev) {
                total_count_++;
                region_counts_[ev.region_id]++;

                // Track their starting location
                citizen_tracker_[ev.citizen.raw_id] = ev.region_id;
            });

            // --- 2. Handle Deaths ---
            registry.subscribe<CitizenDiedEvent>([this](const CitizenDiedEvent& ev) {
                if (total_count_ > 0) {
                    total_count_--;
                }

                auto it = citizen_tracker_.find(ev.citizen.raw_id);
                if (it != citizen_tracker_.end()) {
                    Population::RegionID region = it->second;

                    if (region_counts_[region] > 0) {
                        region_counts_[region]--;
                    }

                    // Cleanup tracker to prevent memory leaks
                    citizen_tracker_.erase(it);
                }
            });

            // --- 3. Handle Relocation ---
            registry.subscribe<CitizenMigratedEvent>([this](const CitizenMigratedEvent& ev) {
                // Remove from old region
                if (region_counts_[ev.from_region] > 0) {
                    region_counts_[ev.from_region]--;
                }

                // Add to new region
                region_counts_[ev.to_region]++;

                // Update tracker
                citizen_tracker_[ev.citizen.raw_id] = ev.to_region;
            });
        }

        /**
         * @brief Gets the live total population of the entire simulation.
         */
        uint32_t get_total_population() const {
            return total_count_;
        }

        /**
         * @brief Gets the live population of a specific spatial region.
         */
        uint32_t get_region_population(Population::RegionID region) const {
            auto it = region_counts_.find(region);
            return it != region_counts_.end() ? it->second : 0;
        }
    };
}