#pragma once

#include <cstdint>

/**
 * @file fixed_point.hpp
 * @brief Implements a deterministic 32-bit Q16.16 fixed-point mathematical type.
 */

/**
 * @struct Fixed32
 * @brief Represents a rational number using 16 bits for the integer and 16 bits for the fraction.
 * @details Guarantees bit-exact arithmetic across all CPU architectures, preventing state drift in deterministic simulations and replays.
 */
struct Fixed32 {
    /**
     * @brief The raw 32-bit internal representation of the fixed-point number.
     */
    int32_t raw_value = 0;

    /**
     * @brief The number of bits dedicated to the fractional component.
     */
    static constexpr int32_t FRACTIONAL_BITS = 16;

    /**
     * @brief The scaling factor used for floating-point conversions.
     */
    static constexpr int64_t SCALE = 1LL << FRACTIONAL_BITS;

    /**
     * @brief Constructs a fixed-point number initialized to zero.
     */
    constexpr Fixed32() = default;

    /**
     * @brief Constructs a fixed-point number from a standard integer.
     * @param val The integer value to convert.
     */
    constexpr explicit Fixed32(int32_t val) : raw_value(val << FRACTIONAL_BITS) {}

    /**
     * @brief Constructs a fixed-point number from a floating-point value.
     * @param val The floating-point value to convert.
     */
    constexpr explicit Fixed32(float val) : raw_value(static_cast<int32_t>(val * static_cast<float>(SCALE))) {}

    /**
     * @brief Creates a Fixed32 directly from a raw pre-shifted bitwise value.
     * @param raw The pre-shifted 32-bit integer.
     * @return A constructed Fixed32 instance.
     */
    static constexpr Fixed32 fromRaw(int32_t raw) {
        Fixed32 result;
        result.raw_value = raw;
        return result;
    }

    /**
     * @brief Converts the fixed-point number back to a truncated integer.
     * @return The integer portion of the value.
     */
    constexpr int32_t toInt() const {
        return raw_value >> FRACTIONAL_BITS;
    }

    /**
     * @brief Converts the fixed-point number to a standard floating-point value.
     * @return The approximate float representation.
     */
    constexpr float toFloat() const {
        return static_cast<float>(raw_value) / static_cast<float>(SCALE);
    }

    /**
     * @brief Performs deterministic fixed-point addition.
     * @param other The value to add.
     * @return The combined fixed-point value.
     */
    constexpr Fixed32 operator+(const Fixed32& other) const {
        return fromRaw(raw_value + other.raw_value);
    }

    /**
     * @brief Performs deterministic fixed-point subtraction.
     * @param other The value to subtract.
     * @return The resulting fixed-point value.
     */
    constexpr Fixed32 operator-(const Fixed32& other) const {
        return fromRaw(raw_value - other.raw_value);
    }

    /**
     * @brief Performs deterministic fixed-point multiplication.
     * @details Temporarily promotes to 64-bit to prevent overflow during bitwise shift.
     * @param other The multiplier.
     * @return The multiplied fixed-point value.
     */
    constexpr Fixed32 operator*(const Fixed32& other) const {
        return fromRaw(static_cast<int32_t>((static_cast<int64_t>(raw_value) * other.raw_value) >> FRACTIONAL_BITS));
    }

    /**
     * @brief Performs deterministic fixed-point division.
     * @details Temporarily promotes to 64-bit and shifts before division to preserve fractional precision. Guards against division by zero.
     * @param other The divisor.
     * @return The divided fixed-point value, or 0 if a division by zero is attempted.
     */
    constexpr Fixed32 operator/(const Fixed32& other) const {
        // Guard against hardware exceptions!
        if (other.raw_value == 0) {
            return Fixed32(0);
        }
        return fromRaw(static_cast<int32_t>((static_cast<int64_t>(raw_value) << FRACTIONAL_BITS) / other.raw_value));
    }

    /**
     * @brief Evaluates strict equality between two fixed-point numbers.
     * @param other The value to compare against.
     * @return True if the internal raw representations match exactly.
     */
    constexpr bool operator==(const Fixed32& other) const { return raw_value == other.raw_value; }

    /**
     * @brief Evaluates strict inequality between two fixed-point numbers.
     * @param other The value to compare against.
     * @return True if the internal raw representations differ.
     */
    constexpr bool operator!=(const Fixed32& other) const { return raw_value != other.raw_value; }

    /**
     * @brief Evaluates if this value is strictly less than another.
     * @param other The value to compare against.
     * @return True if less than the other value.
     */
    constexpr bool operator<(const Fixed32& other) const { return raw_value < other.raw_value; }

    /**
     * @brief Evaluates if this value is strictly greater than another.
     * @param other The value to compare against.
     * @return True if greater than the other value.
     */
    constexpr bool operator>(const Fixed32& other) const { return raw_value > other.raw_value; }

    /**
     * @brief Evaluates if this value is less than or equal to another.
     * @param other The value to compare against.
     * @return True if less than or equal to the other value.
     */
    constexpr bool operator<=(const Fixed32& other) const { return raw_value <= other.raw_value; }

    /**
     * @brief Evaluates if this value is greater than or equal to another.
     * @param other The value to compare against.
     * @return True if greater than or equal to the other value.
     */
    constexpr bool operator>=(const Fixed32& other) const { return raw_value >= other.raw_value; }
};