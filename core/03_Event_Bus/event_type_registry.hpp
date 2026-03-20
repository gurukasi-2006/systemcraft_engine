#pragma once

#include <atomic>

/**
 * @file event_type_registry.hpp
 * @brief Assigns a unique, thread-safe integer ID to each event type at runtime.
 */


using EventID = int;

class EventTypeRegistry {
private:
    /**
     * @brief The master counter that increments every time a NEW event type is seen.
     */
    static inline std::atomic<EventID> event_counter{0};

public:
    /**
     * @brief Retrieves the unique integer ID for a specific event type.
     * * The magic of static local variables ensures that the counter only increments
     * the VERY FIRST time a specific type T is passed into this function.
     * @tparam T The struct/class representing the event.
     * @return EventID The unique integer.
     */
    template<typename T>
    static EventID getID() {

        static EventID unique_id = event_counter++;
        return unique_id;
    }
};