/**
 * @file entity_manager.cpp
 * @brief Implementation of the EntityManager class.
 * * This file contains the logic for translating between the engine's custom
 * EntityID strong type and the underlying EnTT registry.
 * Note: The detailed API documentation for these functions lives in entity_manager.h.
 */
#include "entity_manager.hpp"


EntityManager::EntityManager(entt::registry &reg) : registryref(reg){}

EntityID EntityManager:: createEntity(){
    entt:uint32_t new_raw_id=static_cast<uint32_t>(registryref.create());
    return EntityID{new_raw_id};
}

void EntityManager::destroyEntity(EntityID target){
    registryref.destroy(static_cast<entt::entity>(target.raw_id));
}

bool EntityManager::isValid(EntityID target){
    return registryref.valid(static_cast<entt::entity>(target.raw_id));
}

void EntityManager::clearall(){
    registryref.clear();
}
