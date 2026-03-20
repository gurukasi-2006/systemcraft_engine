#pragma once

#include <string>
#include <unordered_map>
#include "../types/entity_id.hpp"

/**
 * @file tag_manager.h
 * @brief Manages unique string identifiers for specific entities.
 */

/**
 * @class TagManager
 * @brief A two-way dictionary for tagging and retrieving critical entities.
 * * Allows game logic to instantly find unique entities (like "Player" or "MainCamera")
 * without iterating through massive component pools. It guarantees O(1) lookup time.
 */
class TagManager {
private:
    /**
     * @var tag_to_entity
     * @brief Maps a human-readable string tag to a specific EntityID.
     */
    std::unordered_map<std::string, EntityID> tag_to_entity;

    /**
     * @var entity_to_tag
     * @brief Maps a raw entity integer back to its string tag for reverse lookups.
     */
    std::unordered_map<uint32_t, std::string> entity_to_tag;

public:
    /**
     * @brief Assigns a unique string tag to an entity.
     * * If the tag already exists, it will be reassigned to the new entity.
     * @param id The entity receiving the tag.
     * @param tag The unique string name (e.g., "MainCamera").
     */
    void tagEntity(EntityID id, const std::string& tag);

    /**
     * @brief Retrieves an entity by its unique string tag.
     * @param tag The string name to search for.
     * @return EntityID The matching entity. Returns an invalid EntityID if the tag does not exist.
     */
    EntityID getEntityByTag(const std::string& tag);

    /**
     * @brief Removes a tag from an entity, clearing it from the dictionaries.
     * @param id The entity whose tag should be removed.
     */
    void removeTag(EntityID id);

    /**
     * @brief Checks if a specific entity has a tag.
     * @param id The entity to check.
     * @return true if the entity has a tag, false otherwise.
     */
    bool hasTag(EntityID id);
};