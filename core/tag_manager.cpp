/**
 * @file tag_manager.cpp
 * @brief Implementation of the TagManager class.
 * * Handles the dual-dictionary math to ensure tags and entities stay perfectly
 * synchronized in O(1) time without memory leaks.
 */

#include "tag_manager.hpp"

void TagManager::tagEntity(EntityID id, const std::string& tag) {
    if (hasTag(id)) {
        removeTag(id);
    }

    auto it = tag_to_entity.find(tag);
    if (it != tag_to_entity.end()) {
        entity_to_tag.erase(it->second.raw_id);
    }

    tag_to_entity[tag] = id;
    entity_to_tag[id.raw_id] = tag;
}

EntityID TagManager::getEntityByTag(const std::string& tag) {
    auto it = tag_to_entity.find(tag);

    if (it != tag_to_entity.end()) {
        return it->second;
    }

    return EntityID{ static_cast<uint32_t>(entt::null) };
}

void TagManager::removeTag(EntityID id) {
    auto it = entity_to_tag.find(id.raw_id);

    if (it != entity_to_tag.end()) {
        tag_to_entity.erase(it->second);

        entity_to_tag.erase(it);
    }
}

bool TagManager::hasTag(EntityID id) {
    return entity_to_tag.find(id.raw_id) != entity_to_tag.end();
}