//#4th Subsystem

/**
 * @file ecs_view.h
 * @brief Provides a fast, type-safe iterator for querying entities with specific components.
 */

#pragma once

#include<entt/entt.hpp>
#include "../04_Types/entity_id.hpp"

/**
 * @class ECSView
 * @brief A variadic template wrapper around EnTT's view system.
 * * This class allows game systems (like Physics or Traffic) to instantly iterate over
 * every entity in the game that possesses a specific combination of components. It safely
 * intercepts EnTT's raw entities and packages them into the engine's custom EntityID.
 * * @tparam Components... A parameter pack of one or more component types to query for
 * (e.g., ECSView<Position, Velocity>).
 */
template <typename... Components>
class ECSView{

    private:
        /**
         * @var registryref
         * @brief A reference to the master EnTT registry where the actual component pools live.
         */
        entt::registry& registryref;

    public:

        /**
         * @brief Constructs the ECSView and links it to the master registry.
         * @param reg A reference to the master EnTT registry.
         */
        ECSView(entt::registry& reg):registryref(reg){}

        /**
         * @brief Iterates over every entity that has all the requested components.
         * * This function queries the dense component pools and executes a provided callback
         * function on every match. It guarantees O(1) cache-friendly iteration.
         * * @tparam Func The type of the callback function or lambda.
         * @param callback A function to execute on each matching entity. It must accept an
         * EntityID as its first parameter, followed by references to the requested components.
         */
        template <typename Func>
        void each(Func callback){
            auto view=registryref.view<Components...>();
                view.each([&callback](auto raw_entity, auto&... components) {
                EntityID safe_id = { static_cast<uint32_t>(raw_entity) };
                callback(safe_id, components...);
            });
        }
};