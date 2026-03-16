#pragma once

#include <entt/entt.hpp>
#include "types/entity_id.hpp"

/**
 * @file component_handle.h
 * @brief Provides a memory-safe wrapper for accessing component data.
 */

/**
 * @class ComponentHandle
 * @brief A safe reference to a specific component on a specific entity.
 * * * Protects game systems from dangling pointers. If the underlying memory pool
 * resizes and moves data, this handle guarantees it will always fetch the
 * correct, updated memory address by querying the registry on demand.
 * * @tparam T The type of component this handle manages (e.g., Position).
 */
template <typename T>
class ComponentHandle {
private:
    /**
     * @var entity
     * @brief The safe, strongly-typed ID of the entity that owns the component.
     */
    EntityID entity;

    /**
     * @var registry
     * @brief A pointer to the master registry. We use a pointer instead of a reference
     * so that handles can be easily copied and reassigned.
     */
    entt::registry* registry;

public:
    /**
     * @brief Constructs a safe handle to a component.
     * @param target_entity The entity that owns the component.
     * @param reg The master registry where the component lives.
     */
    ComponentHandle(EntityID target_entity, entt::registry& reg)
        : entity(target_entity), registry(&reg) {}

    /**
     * @brief Checks if the entity still has this component attached to it.
     * @return true if the component exists, false if it was removed or destroyed.
     */
    bool isValid() const {
        // We cast our safe ID back to EnTT's raw entity type to ask the registry
        return registry->all_of<T>(static_cast<entt::entity>(entity.raw_id));
    }

    /**
     * @brief Retrieves a direct, up-to-date reference to the component data.
     * * WARNING: You should always call isValid() before calling get() to avoid crashes!
     * @return T& A reference to the actual component data.
     */
    T& get() {
        return registry->get<T>(static_cast<entt::entity>(entity.raw_id));
    }

    /**
     * @brief Overloads the arrow operator (->) for clean pointer-like syntax.
     * * This allows game logic to use `handle->x = 5.0f;` instead of `handle.get().x = 5.0f;`
     * @return T* A pointer to the component data.
     */
    T* operator->() {
        return &registry->get<T>(static_cast<entt::entity>(entity.raw_id));
    }
};