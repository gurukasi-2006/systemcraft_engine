#pragma once

#include <vector>
#include <cstdint>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/fixed_point.hpp"
#include "../../core/04_Types/terrain_type.hpp"
#include "../../core/04_Types/engine_assert.hpp"

#include "grid_data_store.hpp"
#include "elevation_layer.hpp"
#include "water_body_layer.hpp"

/**
 * @file terrain_modifier.hpp
 * @brief API for permanent, player-driven alterations to the procedural world generation.
 * @details Records all changes into a deterministic delta log to allow save files to replay modifications without storing the entire map state.
 */

/**
 * @enum ModificationAction
 * @brief Defines the specific type of terraforming performed.
 */
enum class ModificationAction : uint8_t {
    ClearForest,
    FlattenTerrain,
    ReclaimLand  // Filling in coast/ocean to create buildable land
};

/**
 * @struct TerrainModificationRecord
 * @brief A chronological ledger entry of a single tile alteration.
 */
struct TerrainModificationRecord {
    TileCoord coord;
    ModificationAction action;

    // State storage for replay and potential "undo" mechanics
    Fixed32 old_elevation;
    Fixed32 new_elevation;
    TerrainType old_terrain;
    TerrainType new_terrain;
};

/**
 * @class TerrainModifier
 * @brief Orchestrates cross-layer terrain updates and maintains the modification ledger.
 */
class TerrainModifier {
private:
    /** @brief The delta log of all manual changes applied on top of the procedural seed. */
    std::vector<TerrainModificationRecord> modification_log_;

public:
    TerrainModifier() = default;

    /**
     * @brief Retrieves the chronological log of all terraforming events.
     * @return A read-only reference to the modification log.
     */
    const std::vector<TerrainModificationRecord>& get_modification_log() const {
        return modification_log_;
    }

    /**
     * @brief Clears the log entirely (e.g., when loading a new game).
     */
    void clear_log() {
        modification_log_.clear();
    }

    /**
     * @brief Deforests a tile, converting it to Plains.
     * @param coord The target tile.
     * @param grid The world's primary spatial data store.
     */
    void clear_forest(TileCoord coord, GridDataStore& grid) {
        ENGINE_ASSERT(grid.is_valid(coord), "TileCoord out of bounds in clear_forest.");

        GridCell& cell = grid.get_cell(coord);
        if (cell.terrain == TerrainType::Forest) {
            TerrainModificationRecord record{
                coord,
                ModificationAction::ClearForest,
                cell.elevation, cell.elevation, // Elevation remains unchanged
                TerrainType::Forest, TerrainType::Plains
            };

            cell.terrain = TerrainType::Plains;
            modification_log_.push_back(record);
        }
    }

    /**
     * @brief Forcibly alters the elevation of a tile and updates its biome classification.
     * @param coord The target tile.
     * @param target_elevation The new fixed-point height.
     * @param grid The spatial data store (to update the visual biome).
     * @param elevation The topological layer.
     */
    void flatten_terrain(TileCoord coord, Fixed32 target_elevation, GridDataStore& grid, ElevationLayer& elevation) {
        ENGINE_ASSERT(grid.is_valid(coord) && elevation.is_valid(coord), "TileCoord out of bounds in flatten_terrain.");

        GridCell& cell = grid.get_cell(coord);
        Fixed32 old_elev = elevation.get_elevation(coord);
        TerrainType old_terrain = cell.terrain;

        // Apply spatial changes
        elevation.set_elevation(coord, target_elevation);
        cell.elevation = target_elevation; // Sync the SoA data back to the grid cache

        // NOTE: In a full implementation, you would call TerrainMapper::determine_natural_terrain here
        // to re-classify the biome based on the new elevation. For now, we leave the terrain type as-is
        // unless it was a mountain being leveled, etc.

        TerrainModificationRecord record{
            coord,
            ModificationAction::FlattenTerrain,
            old_elev, target_elevation,
            old_terrain, cell.terrain
        };

        modification_log_.push_back(record);
    }
};