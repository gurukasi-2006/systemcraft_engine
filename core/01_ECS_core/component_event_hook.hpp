#pragma once

#include <entt/entt.hpp>
#include "../types/entity_id.hpp"

/**
 * @file component_event_hook.hpp
 * @brief Manages callbacks for component lifecycle events (add/remove).
 */

/**
 * @class ComponentEventHook
 * @brief Allows systems to react instantly when data is attached or destroyed.
 * * Wraps EnTT's native signal system. Instead of systems constantly polling
 * the registry for changes, they can subscribe to these hooks and let the
 * engine notify them automatically.
 */
class ComponentEventHook {
private:
    /**
     * @var registry
     * @brief Reference to the master registry where the signals are emitted.
     */
    entt::registry& registry;

public:
    /**
     * @brief Constructs the hook manager and links it to the master registry.
     * @param reg The master EnTT registry.
     */
    ComponentEventHook(entt::registry& reg) : registry(reg) {}

    /**
     * @brief Subscribes a class member function to an "On Added" event.
     * * The target function MUST have the signature: void func(entt::registry&, entt::entity)
     * * @tparam Component The component type to listen for (e.g., Position).
     * @tparam Function The member function pointer (e.g., &MySystem::onAdded).
     * @tparam Instance The class type of the listener.
     * @param instance Pointer to the object that owns the function.
     */
    template<typename Component, auto Function, typename Instance>
    void subscribeOnAdd(Instance* instance) {
        registry.on_construct<Component>().template connect<Function>(instance);
    }

    /**
     * @brief Subscribes a class member function to an "On Removed" event.
     * * The target function MUST have the signature: void func(entt::registry&, entt::entity)
     * * @tparam Component The component type to listen for.
     * @tparam Function The member function pointer.
     * @tparam Instance The class type of the listener.
     * @param instance Pointer to the object that owns the function.
     */
    template<typename Component, auto Function, typename Instance>
    void subscribeOnRemove(Instance* instance) {
        registry.on_destroy<Component>().template connect<Function>(instance);
    }
};