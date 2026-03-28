#pragma once

#include <string>
#include "../../core/03_Event_Bus/immediate_dispatcher.hpp"

/**
 * @file generation_progress_reporter.hpp
 * @brief Bridges the synchronous world generation pipeline with the engine's real-time event system.
 * @details Utilizes the ImmediateDispatcher to bypass buffers, ensuring UI updates happen during blocking loops.
 */

namespace WorldGen {

    /**
     * @struct ProgressEvent
     * @brief The data payload containing progress information for the UI or logger.
     */
    struct ProgressEvent {
        /** @brief Completion percentage (0.0f to 100.0f). */
        float percentage;
        /** @brief Human-readable name of the current generation stage. */
        std::string stage_name;
    };

    /**
     * @class GenerationProgressReporter
     * @brief A utility class to report generation progress via the engine's event system.
     */
    class GenerationProgressReporter {
    private:
        /** @brief Reference to the immediate event router. */
        ImmediateDispatcher& dispatcher_;

    public:
        /**
         * @brief Constructs the reporter with a reference to an ImmediateDispatcher.
         * @param dispatcher The dispatcher used to broadcast events immediately.
         */
        explicit GenerationProgressReporter(ImmediateDispatcher& dispatcher) 
            : dispatcher_(dispatcher) {}

        /**
         * @brief Dispatches a ProgressEvent with the given percentage and stage name.
         * @param percent The progress percentage.
         * @param stage The name of the generation stage.
         */
        void report(float percent, const std::string& stage) {
            // Packages the data and routes it directly to subscribers.
            dispatcher_.dispatch(ProgressEvent{percent, stage});
        }
    };
}