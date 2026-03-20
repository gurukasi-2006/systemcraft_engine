#pragma once

#include <entt/entt.hpp>
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include "../types/entity_id.hpp"

/**
 * @file component_serializer.h
 * @brief Manages the conversion of entity components to and from savable formats.
 */

/**
 * @class ComponentSerializer
 * @brief A dictionary of serialization callbacks for saving and loading game states.
 * * By registering custom serialize/deserialize functions for each component,
 * the engine can safely convert complex memory pools into strings (like JSON)
 * for save files or network transmission.
 */
class ComponentSerializer {
private:
    using SerializeFunc = std::function<std::string(entt::registry&, EntityID)>;
    using DeserializeFunc = std::function<void(entt::registry&, EntityID, const std::string&)>;

    /**
     * @var serializers
     * @brief Maps a component's string name to its custom save function.
     */
    std::unordered_map<std::string, SerializeFunc> serializers;

    /**
     * @var deserializers
     * @brief Maps a component's string name to its custom load function.
     */
    std::unordered_map<std::string, DeserializeFunc> deserializers;

    /**
     * @var registered_components
     * @brief Keeps track of all registered component names so we can loop over them.
     */
    std::vector<std::string> registered_components;

public:
    /**
     * @brief Teaches the engine how to save and load a specific component type.
     * @param name The human-readable name of the component (e.g., "Position").
     * @param serialize The function that converts the component to a string.
     * @param deserialize The function that rebuilds the component from a string.
     */
    void registerComponent(const std::string& name, SerializeFunc serialize, DeserializeFunc deserialize);

    /**
     * @brief Gathers all components on an entity and converts them into a single string.
     * @param registry The master EnTT registry.
     * @param entity The entity to save.
     * @return std::string A formatted string containing all the entity's component data.
     */
    std::string serializeEntity(entt::registry& registry, EntityID entity);

    /**
     * @brief Reads a string of data and attaches the rebuilt components to an entity.
     * @param registry The master EnTT registry.
     * @param entity The entity receiving the loaded components.
     * @param data The string containing the saved data.
     */
    void deserializeEntity(entt::registry& registry, EntityID entity, const std::string& data);
};