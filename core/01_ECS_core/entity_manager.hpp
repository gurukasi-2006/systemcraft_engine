#pragma once
#include <vector>
#include <cstdint>
#include <entt/entt.hpp>
#include "../04_Types/entity_id.hpp"

/**
 * @file entity_manager.hpp
 * @brief Manages the lifecycle of all entities in the Systemcraft engine.
 */

/**
 * @class EntityManager
 * @brief Creates, destroys, and safely recycles ECS entities.
 * * Acts as the safe translation layer between the engine's custom EntityID strong type
 * and the underlying EnTT registry. It prevents memory fragmentation by recycling
 * IDs of destroyed entities.
 */
class EntityManager
{
    private:
        /**
         * @var registryref
         * @brief Reference to the master EnTT registry owned by the ECS World Container.
         */
        entt::registry& registryref;

    public:
        /**
         * @brief Constructs the EntityManager and links it to the world state.
         * @param reg A reference to the master EnTT registry.
         */
        EntityManager(entt::registry& reg);

        /**
         * @brief Requests a new entity ID for the simulation.
         * * Prioritizes reusing an ID from the recycled_ids pool. If the pool is empty,
         * it increments and returns the next_id counter.
         * * @return EntityID A safe, strongly-typed wrapper containing the new raw ID.
         */
        EntityID createEntity();

        /**
         * @brief Destroys an entity and reclaims its ID for future use.
         * * Instructs the EnTT registry to destroy the target, which automatically removes
         * all associated components. The raw ID is then pushed to the recycled_ids pool.
         * * @param target The EntityID to be destroyed.
         */
        void destroyEntity(EntityID target);

        /**
         * @brief Checks if a given EntityID is currently alive in the registry.
         * * @param target The EntityID to validate.
         * @return true If the entity exists and has not been destroyed.
         * @return false If the entity is invalid or has already been destroyed.
         */
        bool isValid(EntityID target);

        /**
         * @brief Completely wipes all entities from the registry and resets all ID counters.
         * * Primarily used during engine teardown or when loading a new save file.
         */
        void clearall();

};