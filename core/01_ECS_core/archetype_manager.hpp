//#5th Subsysystem
/**
 * @file archetype_manager.hpp
 * @brief Manages recipes for constructing predefined entity templates (prefabs).
 */

#pragma once

#include<unordered_map>
#include<string>
#include<functional>
#include"entity_manager.hpp"
#include "../04_Types/entity_id.hpp"

/**
 * @class ArchetypeManager
 * @brief Stores and executes recipes to instantly spawn complex entities.
 * * By mapping a string name (like "Vehicle") to a C++ function that attaches
 * the necessary components, this manager allows systems to easily spawn
 * pre-configured entities without duplicating setup code.
 */
class ArchetypeManager{
    private:

        /**
         * @var entity_managerref
         * @brief Reference to the engine's core Entity Manager for minting new IDs.
         */
        EntityManager& entity_managerref;

        /**
         * @var recipes
         * @brief A dictionary mapping archetype names to their construction functions.
         */
        std::unordered_map<std::string,std::function<void(EntityID)>>recipes;

    public:

        /**
         * @brief Constructs the ArchetypeManager and links it to the Entity Manager.
         * @param em A reference to the core EntityManager.
         */
        ArchetypeManager(EntityManager& em);

        /**
         * @brief Teaches the engine a new recipe for an archetype.
         * @param name The unique string identifier for this archetype (e.g., "Citizen").
         * @param recipe A function (or lambda) that takes an EntityID and attaches the required components.
         */
        void registerArchetype(const std::string& name,std::function<void(EntityID)> recipe);

        /**
         * @brief Spawns a new entity and automatically applies the requested archetype recipe.
         * @param name The string identifier of the recipe to execute.
         * @return EntityID The newly minted and fully constructed entity.
         */
        EntityID spawn(const std::string& name);

};


