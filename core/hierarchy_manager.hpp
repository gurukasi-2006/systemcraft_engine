#pragma once

#include <entt/entt.hpp>
#include <vector>
#include "types/entity_id.hpp"

/**
 * @file hierarchy_manager.hpp
 * @brief Manages parent-child relationships between entities.
 */

/**
 * @struct RelationshipComponent
 * @brief Hidden data component used by the engine to track family trees.
 */
struct RelationshipComponent {
    EntityID parent = { static_cast<uint32_t>(entt::null) };
    std::vector<EntityID> children;
};

/**
 * @class HierarchyManager
 * @brief Safely links entities together for scene graphs and complex prefabs.
 * * Allows entities to be grouped so that game logic (like transforms or destruction)
 * can cascade from a parent down to all of its children.
 */
class HierarchyManager {
private:
    /**
     * @var registry
     * @brief Reference to the master registry to attach and read RelationshipComponents.
     */
    entt::registry& registry;

public:
    /**
     * @brief Constructs the HierarchyManager and links it to the registry.
     * @param reg The master EnTT registry.
     */
    HierarchyManager(entt::registry& reg) : registry(reg) {}

    /**
     * @brief Assigns a child entity to a parent entity.
     * * This automatically updates the RelationshipComponent for both entities.
     * @param parent The entity that will own the child.
     * @param child The entity being attached.
     */
    void addChild(EntityID parent, EntityID child);

    /**
     * @brief Unlinks a child from its parent.
     * @param child The entity to detach from its current parent.
     */
    void removeParent(EntityID child);

    /**
     * @brief Retrieves the parent of a specific entity.
     * @param child The entity to query.
     * @return EntityID The parent entity, or an invalid EnTT null ID if it has no parent.
     */
    EntityID getParent(EntityID child);

    /**
     * @brief Retrieves all children attached to a specific parent.
     * @param parent The entity to query.
     * @return std::vector<EntityID> A list of all attached child entities.
     */
    std::vector<EntityID> getChildren(EntityID parent);
};