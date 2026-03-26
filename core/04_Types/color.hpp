#pragma once

#include <cstdint>

/**
 * @file color.hpp
 * @brief Defines a lightweight RGBA data structure and named color constants.
 * @details Acts as the shared visual language between the simulation state and the rendering frontend.
 */

/**
 * @struct Color
 * @brief A 32-bit color representation using 8-bit channels.
 */
struct Color {
    /** @brief Red channel (0-255) */
    uint8_t r = 0;

    /** @brief Green channel (0-255) */
    uint8_t g = 0;

    /** @brief Blue channel (0-255) */
    uint8_t b = 0;

    /** @brief Alpha channel (0-255, where 255 is fully opaque) */
    uint8_t a = 255;

    /**
     * @brief Constructs a default opaque black color.
     */
    constexpr Color() = default;

    /**
     * @brief Constructs a Color from individual 8-bit channel values.
     * @param red Red intensity.
     * @param green Green intensity.
     * @param blue Blue intensity.
     * @param alpha Opacity level (defaults to 255).
     */
    constexpr Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}

    /**
     * @brief Constructs a Color from a 32-bit hex value (0xRRGGBBAA).
     * @param hex The 32-bit packed hexadecimal color code.
     */
    constexpr explicit Color(uint32_t hex)
        : r(static_cast<uint8_t>((hex >> 24) & 0xFF)),
          g(static_cast<uint8_t>((hex >> 16) & 0xFF)),
          b(static_cast<uint8_t>((hex >> 8) & 0xFF)),
          a(static_cast<uint8_t>(hex & 0xFF)) {}

    /**
     * @brief Evaluates strict equality between two colors.
     * @param other The color to compare against.
     * @return True if all channels match exactly.
     */
    constexpr bool operator==(const Color& other) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    /**
     * @brief Evaluates strict inequality between two colors.
     * @param other The color to compare against.
     * @return True if any channel differs.
     */
    constexpr bool operator!=(const Color& other) const {
        return !(*this == other);
    }
};

/**
 * @namespace Colors
 * @brief Pre-calibrated minimalist palette for UI, terrain, and data overlays.
 */
namespace Colors {

    /**
     * @name UI Palette
     * @brief Muted, high-contrast interface defaults.
     */
    ///@{
    constexpr Color Background(0x1E1E1EFF);
    constexpr Color Surface(0x2D2D2DFF);
    constexpr Color TextPrimary(0xE0E0E0FF);
    constexpr Color TextSecondary(0x9E9E9EFF);
    constexpr Color Accent(0x4A90E2FF);
    constexpr Color Error(0xE57373FF);
    ///@}

    /**
     * @name Terrain Overlays
     * @brief Desaturated tones for map data visualization.
     */
    ///@{
    constexpr Color Plains(0x81C784FF);
    constexpr Color Forest(0x388E3CFF);
    constexpr Color Mountain(0x757575FF);
    constexpr Color Water(0x4FC3F7FF);
    constexpr Color Desert(0xFFB74DFF);
    constexpr Color Urban(0x90A4AEFF);
    ///@}

    /**
     * @name Resource Overlays
     * @brief Distinct identification colors for logistics and extraction views.
     */
    ///@{
    constexpr Color Iron(0xB0BEC5FF);
    constexpr Color Coal(0x424242FF);
    constexpr Color Oil(0x212121FF);
    constexpr Color Uranium(0x00E676FF);
    constexpr Color Wheat(0xFFD54FFF);
    ///@}
}