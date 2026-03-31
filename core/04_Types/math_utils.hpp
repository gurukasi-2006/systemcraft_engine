#pragma once

#include <cmath>
#include <algorithm>
#include <cstdint>

/**
 * @file math_utils.hpp
 * @brief High-performance mathematical utility functions for simulation calculations.
 */

/**
 * @namespace MathUtils
 * @brief Encapsulates a hybrid of standard library wrappers and custom fast-math implementations.
 */
namespace MathUtils {

    /**
     * @brief Clamps a value strictly between a minimum and maximum bound.
     * @details Utilizes C++ standard library clamp for optimal compiler instruction generation.
     * @tparam T The numeric type (int, float, double, etc.).
     * @param value The value to evaluate.
     * @param min The absolute minimum boundary.
     * @param max The absolute maximum boundary.
     * @return The clamped value.
     */
    template<typename T>
    constexpr T clamp(T value, T min, T max) {
        return std::clamp(value, min, max);
    }

    /**
     * @brief Linearly interpolates between two values.
     * @details Utilizes C++20 standard library lerp.
     * @param a The starting value.
     * @param b The target value.
     * @param t The interpolation factor [0.0, 1.0].
     * @return The interpolated value.
     */
    constexpr float lerp(float a, float b, float t) {
        return std::lerp(a, b, t);
    }

    /**
     * @brief Remaps a value from one numerical range to another.
     * @details Guards against zero-width input ranges to prevent NaN propagation.
     * @param value The input value to remap.
     * @param in_min The minimum of the input range.
     * @param in_max The maximum of the input range.
     * @param out_min The minimum of the target range.
     * @param out_max The maximum of the target range.
     * @return The remapped value in the target range, or out_min if the input range is zero.
     */
    constexpr float mapRange(float value, float in_min, float in_max, float out_min, float out_max) {
        // Guard against floating-point division by zero!
        if (in_min == in_max) {
            return out_min; 
        }
        return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min);
    }

    /**
     * @brief A high-performance floor calculation avoiding standard library overhead.
     * @details Uses integer casting to bypass IEEE-754 edge-case handling. Ideal for coordinate snapping.
     * @param value The floating-point value to floor.
     * @return The largest integer less than or equal to the value.
     */
    constexpr int32_t fastFloor(float value) {
        int32_t i = static_cast<int32_t>(value);
        return (value < static_cast<float>(i)) ? (i - 1) : i;
    }
}
