#pragma once

#include <fstream>
#include <string>
#include <cstdint>

#include "../05_Terrain/elevation_layer.hpp"
#include "../05_Terrain/moisture_layer.hpp"
#include "../../core/04_Types/tile_coord.hpp"

/**
 * @file heightmap_export.hpp
 * @brief Exports raw terrain data to binary files for external analysis and tuning.
 */

namespace HeightmapExport {

    /**
     * @brief Saves a terrain layer to a binary file.
     * @details Format: [uint32 width] [uint32 height] [float data...]
     * @tparam LayerType Works with ElevationLayer or MoistureLayer.
     * @param layer The terrain data to export.
     * @param filepath Target file destination (e.g., "heightmap.bin").
     * @return true if export succeeded, false otherwise.
     */
    template <typename LayerType>
    inline bool export_to_binary(const LayerType& layer, const std::string& filepath) {
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        uint32_t width = layer.get_width();
        uint32_t height = layer.get_height();

        // 1. Write Header
        file.write(reinterpret_cast<const char*>(&width), sizeof(width));
        file.write(reinterpret_cast<const char*>(&height), sizeof(height));

        // 2. Write Data
        for (uint32_t y = 0; y < height; ++y) {
            for (uint32_t x = 0; x < width; ++x) {
                // Convert Fixed32 to standard float for external tool compatibility
                // We assume Fixed32 has an explicit float conversion or to_float() method.
                float val;
                if constexpr (std::is_same_v<LayerType, ElevationLayer>) {
                    // Use the native toFloat() method for deterministic conversion
                    val = layer.get_elevation({static_cast<int32_t>(x), static_cast<int32_t>(y)}).toFloat();
                } else {
                    val = layer.get_moisture({static_cast<int32_t>(x), static_cast<int32_t>(y)}).toFloat();
                }

                file.write(reinterpret_cast<const char*>(&val), sizeof(val));
            }
        }

        file.close();
        return true;
    }
}