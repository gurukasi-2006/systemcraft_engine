#pragma once

#include <entt/entt.hpp>
#include "entity_manager.hpp"
#include "archetype_manager.hpp"
#include "tag_manager.hpp"
#include "hierarchy_manager.hpp"
#include "component_serializer.hpp"

/**
 * @file ecs_world.h
 * @brief Defines the top-level container for all ECS subsystems.
 */

/**
 * @class ECSWorld
 * @brief The supreme root object of the ECS architecture.
 * * Owns the master registry and all data managers. This single object is passed
 * by reference into all game systems (e.g., RenderSystem, PhysicsSystem)
 * acting as the absolute single source of truth for the game state.
 */
class ECSWorld {
public:
    /**
     * @var registry
     * @brief The master memory pool holding all entities and components.
     * WARNING: This must be declared first so it is initialized before the managers!
     */
    entt::registry registry;

    /**
     * @brief The manager responsible for minting and destroying EntityIDs.
     */
    EntityManager entity_manager;

    /**
     * @brief The manager responsible for spawning complete prefabs (Archetypes).
     */
    ArchetypeManager archetype_manager;

    /**
     * @brief The manager responsible for O(1) string-to-entity dictionary lookups.
     */
    TagManager tag_manager;

    /**
     * @brief The manager responsible for parent-child scene graph linking.
     */
    HierarchyManager hierarchy_manager;

    /**
     * @brief The manager responsible for saving and loading component states.
     */
    ComponentSerializer serializer;

    /**
     * @brief Constructs the ECS World and automatically wires the registry
     * into all the child managers that require it.
     */
    ECSWorld()
        : entity_manager(registry),
          archetype_manager(entity_manager),
          hierarchy_manager(registry) {}
};