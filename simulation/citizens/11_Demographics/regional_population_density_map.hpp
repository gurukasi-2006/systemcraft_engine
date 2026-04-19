#pragma once

#include <entt/entt.hpp>
#include <unordered_map>
#include <vector>
#include <cstdint>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../../../core/03_Event_Bus/subscriber_registry.hpp"
#include "../../../core/03_Event_Bus/event_publisher.hpp"
#include "../../../core/04_Types/tile_coord.hpp"

#include "../09_CitizenSpawner/citizen_spawner.hpp"
#include "../09_CitizenSpawner/age_progression_system.hpp"
#include "../09_CitizenSpawner/migration_executor.hpp"
 // For PositionComponent

/**
 * @file regional_population_density_map.hpp
 * @brief Subsystem 138: Maintains an O(1) per-tile citizen density grid and triggers high-density zoning unlocks.
 */

// --- Safe Forward Declarations for Event Hooks ---
#ifndef CITIZEN_SPAWNED_EVENT_HPP
#define CITIZEN_SPAWNED_EVENT_HPP
struct CitizenSpawnedEvent { EntityID citizen; uint32_t region_id; };
#endif



namespace UrbanPlanning {

    /**
     * @struct OversaturationEvent
     * @brief Fired when a specific tile exceeds its residential carrying capacity.
     */
    struct OversaturationEvent {
        TileCoord tile;
        uint32_t density;
    };

    class RegionalPopulationDensityMap {
    private:
        // Queues to capture events from the bus
        std::vector<EntityID> spawn_queue_;
        std::vector<EntityID> migration_queue_;
        std::vector<EntityID> death_queue_;

        // O(1) Spatial Tracker (Entity ID -> TileCoord)
        // Solves the issue of dead citizens losing their PositionComponent before the event is fully processed
        std::unordered_map<uint32_t, TileCoord> citizen_locations_;

        // The actual Density Grid (Hash of TileCoord -> Citizen Count)
        std::unordered_map<uint64_t, uint32_t> density_grid_;

        // Threshold constant from the design spec
        static constexpr uint32_t OVERSATURATION_THRESHOLD = 8;

        inline uint64_t hash_tile(TileCoord t) const {
            return (static_cast<uint64_t>(static_cast<uint32_t>(t.x)) << 32) |
                   (static_cast<uint64_t>(static_cast<uint32_t>(t.y)));
        }

        inline TileCoord unhash_tile(uint64_t h) const {
            return TileCoord{static_cast<int32_t>(h >> 32), static_cast<int32_t>(h & 0xFFFFFFFF)};
        }

    public:
        /**
         * @brief Binds to the Event Bus to intercept demographic changes instantly.
         */
        RegionalPopulationDensityMap(SubscriberRegistry& registry) {
            registry.subscribe<CitizenSpawnedEvent>([this](const CitizenSpawnedEvent& ev) {
                spawn_queue_.push_back(ev.citizen);
            });

            registry.subscribe<CitizenMigratedEvent>([this](const CitizenMigratedEvent& ev) {
                migration_queue_.push_back(ev.citizen);
            });

            registry.subscribe<CitizenDiedEvent>([this](const CitizenDiedEvent& ev) {
                death_queue_.push_back(ev.citizen);
            });
        }

        /**
         * @brief Processes queued spatial changes and fires zoning events for oversaturated tiles.
         */
        void update(ECSWorld& world, EventPublisher& publisher) {

            // --- 1. Process Deaths (Decrements) ---
            for (auto cit : death_queue_) {
                auto it = citizen_locations_.find(cit.raw_id);
                if (it != citizen_locations_.end()) {
                    uint64_t hash = hash_tile(it->second);
                    if (density_grid_[hash] > 0) {
                        density_grid_[hash]--;
                    }
                    citizen_locations_.erase(it);
                }
            }
            death_queue_.clear();

            // --- 2. Process Spawns (Increments) ---
            for (auto cit : spawn_queue_) {
                entt::entity raw = static_cast<entt::entity>(cit.raw_id);
                if (world.registry.valid(raw) && world.registry.all_of<PositionComponent>(raw)) {
                    TileCoord pos = world.registry.get<PositionComponent>(raw).coord;

                    citizen_locations_[cit.raw_id] = pos;
                    uint64_t hash = hash_tile(pos);
                    density_grid_[hash]++;

                    // Check for Oversaturation
                    if (density_grid_[hash] > OVERSATURATION_THRESHOLD) {
                        publisher.publish(OversaturationEvent{pos, density_grid_[hash]});
                    }
                }
            }
            spawn_queue_.clear();

            // --- 3. Process Migrations (Shifts) ---
            for (auto cit : migration_queue_) {
                entt::entity raw = static_cast<entt::entity>(cit.raw_id);

                // Decrement old location
                auto it = citizen_locations_.find(cit.raw_id);
                if (it != citizen_locations_.end()) {
                    uint64_t old_hash = hash_tile(it->second);
                    if (density_grid_[old_hash] > 0) {
                        density_grid_[old_hash]--;
                    }
                }

                // Increment new location
                if (world.registry.valid(raw) && world.registry.all_of<PositionComponent>(raw)) {
                    TileCoord new_pos = world.registry.get<PositionComponent>(raw).coord;

                    citizen_locations_[cit.raw_id] = new_pos;
                    uint64_t new_hash = hash_tile(new_pos);
                    density_grid_[new_hash]++;

                    // Check for Oversaturation
                    if (density_grid_[new_hash] > OVERSATURATION_THRESHOLD) {
                        publisher.publish(OversaturationEvent{new_pos, density_grid_[new_hash]});
                    }
                }
            }
            migration_queue_.clear();
        }

        /**
         * @brief Utility readout for other systems (like Transport Pathfinders) to check tile congestion.
         */
        uint32_t get_tile_density(TileCoord tile) const {
            uint64_t hash = hash_tile(tile);
            auto it = density_grid_.find(hash);
            return it != density_grid_.end() ? it->second : 0;
        }
    };
}