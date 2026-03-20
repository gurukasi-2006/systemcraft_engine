#pragma once

#include <unordered_map>
#include <vector>
#include <functional>

/**
 * @file wildcard_registry.hpp
 * @brief Allows systems to subscribe to broad categories of events using base classes.
 */

/**
 * @struct ICategoryEvent
 * @brief Any event that needs to be tracked by a Wildcard system must inherit from this.
 */
struct ICategoryEvent {
    virtual ~ICategoryEvent() = default;

    /**
     * @brief Returns the universal ID for this event's broad category (e.g., 1 = Economy, 2 = Military).
     */
    virtual int getCategory() const = 0;
};

class WildcardRegistry {
private:
    std::unordered_map<int, std::vector<std::function<void(const ICategoryEvent&)>>> category_listeners;

public:
    /**
     * @brief Subscribes to EVERY event that belongs to a specific Category ID.
     * @param category_id The broad category to listen to.
     * @param callback The function to execute. Notice it takes the base class!
     */
    void subscribeToCategory(int category_id, std::function<void(const ICategoryEvent&)> callback) {
        category_listeners[category_id].push_back(callback);
    }

    /**
     * @brief Routes a categorized event to all broad wildcard listeners.
     * @param event The polymorphic event payload.
     */
    void dispatchWildcard(const ICategoryEvent& event) {
        int target_category = event.getCategory();

        if (category_listeners.find(target_category) != category_listeners.end()) {
            for (const auto& callback : category_listeners[target_category]) {
                callback(event);
            }
        }
    }
};