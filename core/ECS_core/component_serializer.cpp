/**
 * @file component_serializer.cpp
 * @brief Implementation of the ComponentSerializer class.
 * * Handles the string parsing and dictionary lookups required to convert
 * live ECS memory into savable text blocks and back again.
 */

#include "component_serializer.hpp"

void ComponentSerializer::registerComponent(const std::string& name, SerializeFunc serialize, DeserializeFunc deserialize) {
    serializers[name] = serialize;
    deserializers[name] = deserialize;

    registered_components.push_back(name);
}

std::string ComponentSerializer::serializeEntity(entt::registry& registry, EntityID entity) {
    std::string save_data = "";

    for (const auto& name : registered_components) {
        std::string comp_data = serializers[name](registry, entity);

        if (!comp_data.empty()) {
            save_data += "[" + name + "]:" + comp_data + ";\n";
        }
    }

    return save_data;
}

void ComponentSerializer::deserializeEntity(entt::registry& registry, EntityID entity, const std::string& data) {
    size_t current_pos = 0;

    while (current_pos < data.length()) {
        size_t name_start = data.find('[', current_pos);
        if (name_start == std::string::npos) break;

        size_t name_end = data.find("]:", name_start);
        if (name_end == std::string::npos) break;

        size_t data_end = data.find(";\n", name_end);
        if (data_end == std::string::npos) break;

        std::string name = data.substr(name_start + 1, name_end - name_start - 1);
        std::string comp_data = data.substr(name_end + 2, data_end - name_end - 2);

        auto it = deserializers.find(name);
        if (it != deserializers.end()) {
            it->second(registry, entity, comp_data);
        }

        current_pos = data_end + 2;
    }
}