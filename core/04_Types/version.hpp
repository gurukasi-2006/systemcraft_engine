#pragma once

#include <cstdint>
#include <string_view>

/**
 * @file version.hpp
 * @brief Defines compile-time versioning constants for the engine and save formats.
 * @details Used by the serialization system to gracefully reject or migrate incompatible save files.
 */

/**
 * @namespace Version
 * @brief Encapsulates all global versioning data for the Systemcraft engine.
 */
namespace Version {

    /** * @brief The structural version of the save file format.
     * @details CRITICAL: Must be incremented whenever ECS component layouts or core serialization arrays change.
     */
    constexpr uint32_t SAVE_FORMAT_VERSION = 1;

    /** @brief The major version of the engine. Incremented on massive architectural changes. */
    constexpr uint32_t GAME_VERSION_MAJOR = 0;

    /** @brief The minor version. Incremented for feature additions and new subsystems. */
    constexpr uint32_t GAME_VERSION_MINOR = 1;

    /** @brief The patch version. Incremented for bug fixes and balance tweaks. */
    constexpr uint32_t GAME_VERSION_PATCH = 0;

    /** * @brief A human-readable string representation of the current engine version.
     * @details Useful for logging, UI rendering, and crash reports.
     */
    constexpr std::string_view GAME_VERSION_STRING = "0.1.0-alpha";

}