#pragma once

#include <cstdint>

/**
 * @file time_constants.hpp
 * @brief Defines the global compile-time constants for the engine's internal tick-based temporal resolution.
 * @details Acts as the single source of truth for all duration, scheduling, and chronological math across all subsystems.
 */

/**
 * @namespace TimeConstants
 * @brief Encapsulates all discrete tick-to-chronological time conversions.
 */
namespace TimeConstants {

    /**
     * @brief The atomic unit of chronological simulation time. Represents exactly one in-game hour.
     */
    constexpr uint64_t TICKS_PER_HOUR = 1;

    /**
     * @brief The number of ticks required to complete one full 24-hour day-night cycle.
     */
    constexpr uint64_t TICKS_PER_DAY = TICKS_PER_HOUR * 24;

    /**
     * @brief The number of ticks in a standard 30-day simulation month.
     * @details Evaluates to 720 ticks. Safely mirrors the Sim::TICKS_PER_GAME_MONTH economic constant.
     */
    constexpr uint64_t TICKS_PER_MONTH = TICKS_PER_DAY * 30;

    /**
     * @brief The number of ticks in a standard 12-month simulation year.
     * @details Evaluates to 8640 ticks. Used for annual taxation, depreciation, and census calculations.
     */
    constexpr uint64_t TICKS_PER_YEAR = TICKS_PER_MONTH * 12;

}