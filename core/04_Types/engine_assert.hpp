#pragma once

#include <iostream>
#include <cstdlib>

/**
 * @file engine_assert.hpp
 * @brief Defines compile-time validation macros that halt execution on invariant violations in Debug builds.
 * @details All macros evaluate to a no-op (zero overhead) in Release builds (when NDEBUG is defined).
 */

#ifndef NDEBUG

/**
 * @brief The core engine assertion macro. Halts execution and prints diagnostic data if the condition is false.
 * @param condition The boolean expression that must evaluate to true.
 * @param message A string literal describing the failure.
 */
#define ENGINE_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            std::cerr << "\n[SYSTEMCRAFT FATAL ASSERTION]\n" \
                      << "File: " << __FILE__ << "\n" \
                      << "Line: " << __LINE__ << "\n" \
                      << "Condition: " << #condition << "\n" \
                      << "Message: " << (message) << "\n" \
                      << "Execution halted.\n" << std::endl; \
            std::abort(); \
        } \
    } while (false)

#else

// In Release builds, the condition is evaluated inside a sizeof() to prevent
// "unused variable" compiler warnings, but generates zero executable machine code.
#define ENGINE_ASSERT(condition, message) do { (void)sizeof(condition); } while(false)

#endif

/**
 * @brief Asserts that an EntityID wrapper points to a valid, active ECS entity.
 * @details Relies on the EntityID wrapper exposing an IsNull() method.
 * @param entity The EntityID wrapper to check.
 */
#define ASSERT_VALID_ENTITY(entity) \
    ENGINE_ASSERT(!(entity).IsNull(), "Attempted to process an invalid or null EntityID.")

/**
 * @brief Asserts that a numerical value falls strictly within a specified inclusive range.
 * @param value The value to evaluate.
 * @param min_val The minimum permitted boundary.
 * @param max_val The maximum permitted boundary.
 */
#define ASSERT_IN_RANGE(value, min_val, max_val) \
    ENGINE_ASSERT(((value) >= (min_val)) && ((value) <= (max_val)), "Value exceeds permitted boundaries.")

/**
 * @brief Asserts that an array index is strictly less than the maximum size of the array.
 * @param index The index being accessed.
 * @param array_size The total capacity of the array.
 */
#define ASSERT_VALID_INDEX(index, array_size) ENGINE_ASSERT((index) >= 0 && (index) < (array_size), "Array index out of bounds.")
