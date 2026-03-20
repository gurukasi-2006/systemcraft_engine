//#5th subsystem
/**
 * @file archetype_manager.cpp
 * @brief Implementation of the ArchetypeManager class.
 * * This file contains the dictionary lookup logic for registering and executing
 * component recipes (prefabs) to construct complex entities instantly.
 * Note: The detailed API documentation for these functions lives in archetype_manager.h.
 */

#include"archetype_manager.hpp"

ArchetypeManager::ArchetypeManager(EntityManager& em):entity_managerref(em){}

void ArchetypeManager::registerArchetype(const std::string& name,std::function<void(EntityID)> recipe){
    recipes[name]=recipe;
}

EntityID ArchetypeManager::spawn(const std::string& name){
    EntityID new_entity=entity_managerref.createEntity();
    auto it=recipes.find(name);
    if(it !=recipes.end()){
        it->second(new_entity);
    }
    return new_entity;
}