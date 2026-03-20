#pragma once

#include <vector>
#include <functional>
#include "event_publisher.hpp"
#include "../02_Tick_Scheduler/tick_counter.hpp"

/**
 * @file event_replay_system.hpp
 * @brief Acts as an armored DVR for the Event Bus, allowing deterministic recording and playback.
 */

enum class ReplayState { OFF, RECORDING, REPLAYING };

class EventReplaySystem {
private:
    /**
     * @brief A perfectly preserved snapshot of an event in time.
     */
    struct RecordedEvent {
        uint64_t tick;
        std::function<void()> playback_action;
    };

    std::vector<RecordedEvent> tape;
    ReplayState current_state = ReplayState::OFF;

    EventPublisher& global_publisher;
    const TickCounter* clock;

    size_t playback_index = 0;

public:
    /**
     * @brief Hooks the DVR up to the engine's master clock and publisher.
     */
    EventReplaySystem(EventPublisher& pub, const TickCounter* clk)
        : global_publisher(pub), clock(clk) {}

    /**
     * @brief Switches the engine's temporal mode.
     */
    void setState(ReplayState state) {
        current_state = state;
        if (state == ReplayState::REPLAYING) playback_index = 0;
        if (state == ReplayState::RECORDING) {
            tape.clear();
            playback_index = 0;
        }
    }

    ReplayState getState() const { return current_state; }

    /**
     * @brief Proxy publisher. Systems call this INSTEAD of the raw EventPublisher.
     * @tparam T The specific event struct.
     * @param event The data payload.
     */
    template<typename T>
    void publish(const T& event) {

        if (current_state == ReplayState::RECORDING) {
            T snapshot = event;
            tape.push_back({
                clock->get(),
                [this, snapshot]() { global_publisher.publish(snapshot); }
            });
        }


        if (current_state != ReplayState::REPLAYING) {
            global_publisher.publish(event);
        }
    }

    /**
     * @brief Injects historical events into the live engine.
     * * MUST be called exactly once per frame (e.g., inside an Early-Update Phase).
     */
    void playFrame() {
        if (current_state == ReplayState::REPLAYING) {
            uint64_t current_tick = clock->get();

            while (playback_index < tape.size() && tape[playback_index].tick == current_tick) {
                tape[playback_index].playback_action();
                playback_index++;
            }
        }
    }

    /**
     * @brief Swaps the temporal anchor of the DVR (useful for rewinding time).
     */
    void setClock(const TickCounter* new_clock) {
        clock = new_clock;
    }
};