//#5th subsystem
/**
 * @file archetype_manager.cpp
 * @brief Implementation of the ArchetypeManager class.
 * * This file contains the dictionary lookup logic for registering and executing
 * component recipes (prefabs) to construct complex entities instantly.
 * Note: The detailed API documentation for these functions lives in archetype_manager.hpp.
 */

#include "archetype_manager.hpp"
#include <iostream>

ArchetypeManager::ArchetypeManager(EntityManager& em) : entity_managerref(em) {}

void ArchetypeManager::registerArchetype(const std::string& name, std::function<void(EntityID)> recipe) {
    recipes[name] = recipe;
}

EntityID ArchetypeManager::spawn(const std::string& name) {
    auto it = recipes.find(name);

    // Guard: Prevent the creation of "Ghost Entities"
    if (it == recipes.end()) {
        std::cerr << "[WARNING] ArchetypeManager: Attempted to spawn unregistered archetype '" << name << "'. Returning Null Entity.\n";
        return EntityID{ static_cast<uint32_t>(entt::null) };
    }

    // Valid recipe found, proceed with instantiation
    EntityID new_entity = entity_managerref.createEntity();
    it->second(new_entity);

    return new_entity;
}
