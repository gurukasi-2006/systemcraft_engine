/**
 * @file hierarchy_manager.cpp
 * @brief Implementation of the HierarchyManager class.
 * * Handles the safe assignment and removal of parent-child entity links,
 * ensuring that the engine's scene graph never contains orphaned pointers.
 */

#include "hierarchy_manager.hpp"
#include <algorithm>

void HierarchyManager::addChild(EntityID parent, EntityID child) {
    auto parent_ent = static_cast<entt::entity>(parent.raw_id);
    auto child_ent = static_cast<entt::entity>(child.raw_id);

    EntityID current_parent = getParent(child);
    if (current_parent.raw_id != static_cast<uint32_t>(entt::null)) {
        removeParent(child);
    }

    auto& parent_rel = registry.get_or_emplace<RelationshipComponent>(parent_ent);
    parent_rel.children.push_back(child);

    auto& child_rel = registry.get_or_emplace<RelationshipComponent>(child_ent);
    child_rel.parent = parent;
}

void HierarchyManager::removeParent(EntityID child) {
    auto child_ent = static_cast<entt::entity>(child.raw_id);

    if (!registry.all_of<RelationshipComponent>(child_ent)) return;

    auto& child_rel = registry.get<RelationshipComponent>(child_ent);
    EntityID parent = child_rel.parent;

    if (parent.raw_id != static_cast<uint32_t>(entt::null)) {
        auto parent_ent = static_cast<entt::entity>(parent.raw_id);

        if (registry.all_of<RelationshipComponent>(parent_ent)) {
            auto& parent_rel = registry.get<RelationshipComponent>(parent_ent);

            parent_rel.children.erase(
                std::remove_if(parent_rel.children.begin(), parent_rel.children.end(),
                    [&](const EntityID& e) { return e.raw_id == child.raw_id; }),
                parent_rel.children.end()
            );
        }

        child_rel.parent = { static_cast<uint32_t>(entt::null) };
    }
}

EntityID HierarchyManager::getParent(EntityID child) {
    auto child_ent = static_cast<entt::entity>(child.raw_id);

    if (registry.all_of<RelationshipComponent>(child_ent)) {
        return registry.get<RelationshipComponent>(child_ent).parent;
    }

    return { static_cast<uint32_t>(entt::null) };
}

std::vector<EntityID> HierarchyManager::getChildren(EntityID parent) {
    auto parent_ent = static_cast<entt::entity>(parent.raw_id);

    if (registry.all_of<RelationshipComponent>(parent_ent)) {
        return registry.get<RelationshipComponent>(parent_ent).children;
    }

    return {};
}