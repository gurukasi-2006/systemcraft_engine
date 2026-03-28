#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/fixed_point.hpp"
#include "../../core/04_Types/terrain_type.hpp"
#include "../05_Terrain/grid_data_store.hpp"
#include "../05_Terrain/water_body_layer.hpp"
#include "../05_Terrain/tile_neighbor_query.hpp"

/**
 * @file starting_region_selector.hpp
 * @brief Evaluates and ranks the world map to find the most strategically viable starting locations.
 */

namespace StartingRegionSelector {

    /**
     * @struct CandidateRegion
     * @brief A data container for a scored starting location.
     */
    struct CandidateRegion {
        TileCoord coord;
        Fixed32 score;
    };

    /**
     * @brief Scans the world and returns the top N starting locations based on habitability.
     * @param grid The world's primary terrain and biome data.
     * @param water The hydrological map (used to find rivers and oceans).
     * @param num_candidates The number of starting locations to return.
     * @return A sorted vector of the best coordinates (highest score first).
     */
    inline std::vector<CandidateRegion> find_best_starts(
        const GridDataStore& grid,
        const WaterBodyLayer& water,
        int32_t num_candidates)
    {
        int32_t w = static_cast<int32_t>(grid.get_width());
        int32_t h = static_cast<int32_t>(grid.get_height());

        std::vector<CandidateRegion> valid_candidates;
        // Pre-allocate to avoid aggressive heap reallocations during the sweep
        valid_candidates.reserve(w * h / 10);

        for (int32_t y = 0; y < h; ++y) {
            for (int32_t x = 0; x < w; ++x) {
                TileCoord current{x, y};
                const GridCell& cell = grid.get_cell(current);

                // 1. Hard Vetoes: You cannot spawn ON the ocean, ON a mountain, or IN a lake.
                if (cell.terrain == TerrainType::Ocean ||
                    cell.terrain == TerrainType::Mountain ||
                    water.get_type(current) != WaterBodyType::None) {
                    continue;
                }

                Fixed32 score(10.0f); // Base score for existing on land

                // 2. Terrain Bonuses
                if (cell.terrain == TerrainType::Plains) {
                    score = score + Fixed32(5.0f); // Prime building real estate
                } else if (cell.terrain == TerrainType::Forest) {
                    score = score + Fixed32(2.0f); // Good for timber, but requires clearing
                }

                // 3. Adjacency Bonuses (Water & Coast)
                auto neighbors = TileNeighborQuery::get_all(current, w, h);
                bool has_river = false;
                bool has_coast = false;

                for (int8_t i = 0; i < neighbors.count; ++i) {
                    TileCoord n = neighbors.coordinates[i];

                    if (water.get_type(n) == WaterBodyType::River) {
                        has_river = true;
                    }
                    if (grid.get_cell(n).terrain == TerrainType::Coast ||
                        grid.get_cell(n).terrain == TerrainType::Ocean) {
                        has_coast = true;
                    }
                }

                if (has_river) score = score + Fixed32(8.0f); // Fresh water is critical
                if (has_coast) score = score + Fixed32(4.0f); // Maritime trade access

                valid_candidates.push_back({current, score});
            }
        }

        // 4. Sort the candidates (Highest score first)
        // CRITICAL: We tie-break using X and Y coordinates to guarantee cross-platform determinism!
        std::sort(valid_candidates.begin(), valid_candidates.end(),
            [](const CandidateRegion& a, const CandidateRegion& b) {
                if (a.score != b.score) return a.score > b.score;
                if (a.coord.x != b.coord.x) return a.coord.x < b.coord.x;
                return a.coord.y < b.coord.y;
            });

        // 5. Truncate to the requested number of candidates
        if (valid_candidates.size() > static_cast<size_t>(num_candidates)) {
            valid_candidates.resize(num_candidates);
        }

        return valid_candidates;
    }
}