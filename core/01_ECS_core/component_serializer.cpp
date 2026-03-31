/**
 * @file component_serializer.cpp
 * @brief Implementation of the ComponentSerializer class.
 */

#include "component_serializer.hpp"
#include <sstream>
#include <iostream>

void ComponentSerializer::registerComponent(
    const std::string& name,
    std::function<std::string(entt::registry&, EntityID)> save_fn,
    std::function<void(entt::registry&, EntityID, const std::string&)> load_fn)
{
    // Prevent duplicate entries in the name list if registered multiple times
    if (serializers.find(name) == serializers.end()) {
        registered_components.push_back(name);
    }
    serializers[name] = save_fn;
    deserializers[name] = load_fn;
}

std::string ComponentSerializer::serializeEntity(entt::registry& registry, EntityID entity) {
    std::ostringstream out;
    out << "Entity:" << entity.raw_id << "\n";

    for (const auto& name : registered_components) {
        auto it = serializers.find(name);

        //Ensure the key exists AND the function is not null before calling
        if (it != serializers.end() && it->second) {
            std::string comp_data = it->second(registry, entity);
            if (!comp_data.empty()) {
                out << "[" << name << "]:" << comp_data << "\n";
            }
        } else {
            std::cerr << "[WARNING] ComponentSerializer: Missing or null serializer for '" << name << "'.\n";
        }
    }
    return out.str();
}

void ComponentSerializer::deserializeEntity(entt::registry& registry, EntityID entity, const std::string& data) {
    std::istringstream in(data);
    std::string line;

    while (std::getline(in, line)) {
        if (line.empty() || line.find("Entity:") == 0) continue;

        size_t bracket_end = line.find("]:");
        if (line[0] == '[' && bracket_end != std::string::npos) {
            std::string name = line.substr(1, bracket_end - 1);
            std::string comp_data = line.substr(bracket_end + 2);

            auto it = deserializers.find(name);

            // Prevents bad_function_call crash on outdated save files
            if (it != deserializers.end() && it->second) {
                it->second(registry, entity, comp_data);
            } else {
                std::cerr << "[WARNING] ComponentSerializer: No deserializer found for saved component '" << name << "'. Ignoring.\n";
            }
        }
    }
}
