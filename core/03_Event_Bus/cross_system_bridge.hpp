#pragma once

#include <functional>
#include "subscriber_registry.hpp"
#include "event_publisher.hpp"

/**
 * @file cross_system_bridge.hpp
 * @brief Translates and routes events across strict module boundaries without mutual includes.
 */

class CrossSystemBridge {
private:
    SubscriberRegistry& registry;
    EventPublisher& publisher;

public:
    /**
     * @brief Links the bridge to the engine's master phonebook and megaphone.
     */
    CrossSystemBridge(SubscriberRegistry& reg, EventPublisher& pub)
        : registry(reg), publisher(pub) {}

    /**
     * @brief Creates a permanent translation route between two isolated event types.
     * @tparam SourceEvent The event emitted by Domain A (e.g., Economy).
     * @tparam DestEvent The event understood by Domain B (e.g., Audio).
     * @param translator A lambda that converts the Source data into the Dest format.
     */
    template<typename SourceEvent, typename DestEvent>
    void createBridge(std::function<DestEvent(const SourceEvent&)> translator) {

        registry.subscribe<SourceEvent>([this, translator](const SourceEvent& incoming_event) {

            DestEvent outgoing_event = translator(incoming_event);

            publisher.publish(outgoing_event);
        });
    }
};