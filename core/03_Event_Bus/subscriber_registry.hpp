#pragma once

#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>
#include "event_type_registry.hpp"

/**
 * @file subscriber_registry.hpp
 * @brief Stores lists of callback functions mapped to specific event types.
 */

/**
 * @class ICallbackContainer
 * @brief The purely virtual base class allowing us to store different function types in one map.
 */
class ICallbackContainer {
public:
    virtual ~ICallbackContainer() = default;
};

/**
 * @class CallbackContainer
 * @brief The strictly-typed wrapper that holds the actual std::function callbacks.
 */
template<typename T>
class CallbackContainer : public ICallbackContainer {
public:
    std::vector<std::function<void(const T&)>> callbacks;
};

/**
 * @class SubscriberRegistry
 * @brief The master phonebook of who wants to listen to what.
 */
class SubscriberRegistry {
private:
    std::unordered_map<EventID, std::unique_ptr<ICallbackContainer>> subscribers;

public:
    /**
     * @brief Registers a function to be called whenever event type T is dispatched.
     * @tparam T The specific event struct.
     * @param callback The function/lambda to execute.
     */
    template<typename T>
    void subscribe(std::function<void(const T&)> callback) {
        EventID id = EventTypeRegistry::getID<T>();

        if (subscribers.find(id) == subscribers.end()) {
            subscribers[id] = std::make_unique<CallbackContainer<T>>();
        }

        auto* container = static_cast<CallbackContainer<T>*>(subscribers[id].get());
        container->callbacks.push_back(callback);
    }

    /**
     * @brief Retrieves all registered callbacks for a specific event type.
     */
    template<typename T>
    const std::vector<std::function<void(const T&)>>& getCallbacks() {
        EventID id = EventTypeRegistry::getID<T>();

        if (subscribers.find(id) == subscribers.end()) {
            subscribers[id] = std::make_unique<CallbackContainer<T>>();
        }

        auto* container = static_cast<CallbackContainer<T>*>(subscribers[id].get());
        return container->callbacks;
    }
};