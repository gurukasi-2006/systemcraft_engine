#pragma once

#include <unordered_map>
#include "event_type_registry.hpp"
#include "event_publisher.hpp"

/**
 * @file event_throttler.hpp
 * @brief Intercepts events before they hit the Publisher, dropping them if a per-tick limit is reached.
 */

class EventThrottler {
private:
    EventPublisher& global_publisher;


    std::unordered_map<EventID, uint32_t> limits;


    std::unordered_map<EventID, uint32_t> current_counts;

public:
    /**
     * @brief Links the throttler to your master megaphone.
     */
    EventThrottler(EventPublisher& pub) : global_publisher(pub) {}

    /**
     * @brief Sets the maximum number of times a specific event can be published in one frame.
     * @tparam T The specific event struct.
     * @param max_per_tick The limit.
     */
    template<typename T>
    void setLimit(uint32_t max_per_tick) {
        EventID id = EventTypeRegistry::getID<T>();
        limits[id] = max_per_tick;
    }

    /**
     * @brief Attempts to publish an event. Drops it silently if the limit is exceeded.
     * @tparam T The specific event struct.
     * @param event The data payload.
     */
    template<typename T>
    void publish(const T& event) {
        EventID id = EventTypeRegistry::getID<T>();


        if (limits.find(id) != limits.end()) {

            if (current_counts[id] >= limits[id]) {
                return;
            }

            current_counts[id]++;
        }

        global_publisher.publish(event);
    }

    /**
     * @brief Resets the internal counters.
     * * MUST be called at the very end of your master game loop (usually in the LateUpdateBucket).
     */
    void resetCounters() {
        current_counts.clear();
    }
};