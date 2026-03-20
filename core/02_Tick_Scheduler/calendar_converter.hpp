#pragma once

#include <cstdint>
#include <string>

/**
 * @file calendar_converter.hpp
 * @brief Translates raw engine ticks into human-readable in-game dates.
 */

/**
 * @struct GameDate
 * @brief A human-readable representation of in-game time.
 */
struct GameDate {
    uint32_t year;
    uint32_t month;
    uint32_t day;
    uint32_t hour;
};

class CalendarConverter {
private:
    /**
     * @brief How many engine ticks equal one in-game hour.
     */
    uint64_t ticks_per_hour;

public:
    /**
     * @brief Configures the speed of the calendar.
     * @param ticks_for_one_hour How many master ticks it takes to advance the clock by 1 hour.
     */
    CalendarConverter(uint64_t ticks_for_one_hour = 60)
        : ticks_per_hour(ticks_for_one_hour) {
        if (ticks_per_hour == 0) ticks_per_hour = 1;
    }

    /**
     * @brief Converts an absolute tick count into a formatted GameDate struct.
     * * Assumes a standardized simulation calendar: 24h days, 30-day months, 12-month years.
     * @param total_ticks The current master clock tick.
     * @return GameDate The parsed date (1-indexed for Y/M/D, 0-indexed for Hours).
     */
    GameDate convert(uint64_t total_ticks) const {
        GameDate date;

        uint64_t total_hours = total_ticks / ticks_per_hour;

        date.hour = static_cast<uint32_t>(total_hours % 24);

        uint64_t total_days = total_hours / 24;


        date.day = static_cast<uint32_t>((total_days % 30) + 1);


        uint64_t total_months = total_days / 30;


        date.month = static_cast<uint32_t>((total_months % 12) + 1);


        date.year = static_cast<uint32_t>((total_months / 12) + 1);

        return date;
    }
};