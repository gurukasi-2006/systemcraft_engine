#pragma once

#include <entt/entt.hpp>
#include "../04_Types/entity_id.hpp"
/**
 * @file query_builder.hpp
 * @brief Provides a fluent, chainable API for iterating over specific entities.
 */

/**
 * @class QueryBuilder
 * @brief Constructs high-performance queries to filter entities.
 * * Allows game systems to request entities that match an exact recipe of components
 * (e.g., has A and B, but lacks C) and iterates over them in O(1) continuous memory.
 */
class QueryBuilder {
private:
    /**
     * @var registry
     * @brief The master registry to query against.
     */
    entt::registry& registry;

public:
    /**
     * @brief Constructs the query builder.
     * @param reg The master EnTT registry.
     */
    QueryBuilder(entt::registry& reg) : registry(reg) {}

    /**
     * @class Query
     * @brief The internal builder object returned by .with<>()
     */
    template<typename... Includes>
    class Query {
    private:
        entt::registry& reg;

    public:
        Query(entt::registry& r) : reg(r) {}

        /**
         * @brief Executes the query, strictly EXCLUDING entities with specific components.
         * * Unpacks the components and passes them directly into your custom loop logic.
         * @tparam Excludes The components the entity MUST NOT have.
         * @param func A lambda function to execute on every matching entity.
         */
        template<typename... Excludes, typename Func>
        void without(Func&& func) {
            auto view = reg.view<Includes...>(entt::exclude<Excludes...>);
            for (auto entity : view) {
                // Instantly grab the requested components and pass them to the user's logic!
                func(EntityID{ static_cast<uint32_t>(entity) }, reg.get<Includes>(entity)...);
            }
        }

        /**
         * @brief Executes the query with only the included components.
         * @param func A lambda function to execute on every matching entity.
         */
        template<typename Func>
        void build(Func&& func) {
            auto view = reg.view<Includes...>();
            for (auto entity : view) {
                // Instantly grab the requested components and pass them to the user's logic!
                func(EntityID{ static_cast<uint32_t>(entity) }, reg.get<Includes>(entity)...);
            }
        }
    };

    /**
     * @brief Starts building a query by defining the required components.
     * @tparam Includes The components the entity MUST have.
     * @return A Query object ready to be executed or further filtered.
     */
    template<typename... Includes>
    Query<Includes...> with() {
        return Query<Includes...>(registry);
    }
};