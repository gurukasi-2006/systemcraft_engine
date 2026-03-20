//#3 subsystem

/**
 * @file component_pool.h
 * @brief Provides a dense, contiguous array for storing all instances of a single component type.
 */

#pragma once

#include <entt/entt.hpp>
#include "../04_Types/entity_id.hpp"

/**
 * @class ComponentPool
 * @brief A type-safe wrapper around EnTT's component storage.
 * * This template class guarantees cache locality by storing all instances of a specific
 * component type (like Position or Health) in a tightly packed array. It translates
 * the engine's custom EntityID strong type into raw EnTT entities for safe access.
 * * @tparam T The component type this pool manages (e.g., struct Position).
 */
template<typename T>
class ComponentPool{

    private:

        /**
         * @var registryref
         * @brief A reference to the master EnTT registry where the actual data lives.
         */
        entt::registry& registryref;
    public:

        /**
         * @brief Constructs the ComponentPool and links it to the master registry.
         * @param reg A reference to the master EnTT registry.
         */
        ComponentPool(entt::registry& reg) : registryref(reg){}


        /**
         * @brief Attaches a new component of type T to the specified entity.
         * @param target The EntityID receiving the component.
         * @param component_data The actual data struct being copied into the pool.
         * @return T& A reference to the newly inserted component data in the dense array.
         */
        T& insert(EntityID target, T component_data)
        {
            auto entt_id=static_cast<entt::entity>(target.raw_id);
            return registryref.emplace<T>(entt_id,component_data);
        }


        /**
         * @brief Retrieves a reference to an entity's component for reading or modification.
         * @param target The EntityID whose component is being accessed.
         * @return T& A reference to the component data.
         */
        T& get(EntityID target){
            auto entt_id=static_cast<entt::entity>(target.raw_id);
            return registryref.get<T>(entt_id);
        }

        /**
         * @brief Removes the component from the entity.
         * * EnTT automatically swaps the last element of the array into the empty slot
         * to maintain perfect memory contiguity with zero fragmentation.
         * * @param target The EntityID losing the component.
         */
        void remove(EntityID target){
            auto entt_id=static_cast<entt::entity>(target.raw_id);
            registryref.remove<T>(entt_id);
        }

        /**
         * @brief Checks if a specific entity currently owns this type of component.
         * @param target The EntityID to check.
         * @return true If the entity possesses the component.
         * @return false If the entity does not possess the component.
         */
        bool has(EntityID target){
            auto entt_id=static_cast<entt::entity>(target.raw_id);
            return registryref.all_of<T>(entt_id);

        }

};