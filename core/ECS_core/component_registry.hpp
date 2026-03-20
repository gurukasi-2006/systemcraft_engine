/**
 * @file component_registry.h
 * @brief Maps component types to unique numeric IDs at startup.
 */
#pragma once


#include<cstdint>

/**
 * @class ComponentRegistry
 * @brief A purely static class that generates and stores a unique ID for every component type.
 * * By using templates and static local variables, this registry assigns a unique
 * uint32_t ID to any struct or class passed to it (e.g., Position, Health).
 * This allows the engine to store and retrieve components generically without
 * knowing their concrete type ahead of time.
 */
class ComponentRegistry{

private:

    /**
     * @var next_type_id
     * @brief A hidden, auto-incrementing counter tracking the next available component ID.
     * * Initialized inline (C++20 feature) to avoid needing a separate .cpp implementation file.
     */
    static inline uint32_t next_type_id=0;

public:

    /**
     * @brief Retrieves the unique numeric ID for a specific component type.
     * * The first time this is called for a new type, it assigns it the current next_type_id
     * and increments the counter. Every subsequent call for that same type instantly
     * returns the assigned ID with O(1) performance.
     * * @tparam T The component type (e.g., struct Position, struct Health) to look up.
     * @return uint32_t The unique ID permanently mapped to type T.
     */
    template<typename T>
    static uint32_t getTypeID(){
        static uint32_t id=next_type_id++;
        return id;
    }

};