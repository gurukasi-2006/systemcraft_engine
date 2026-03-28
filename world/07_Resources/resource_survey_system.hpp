#pragma once

#include <vector>
#include <cstdint>

#include "../../core/04_Types/tile_coord.hpp"

/**
 * @file resource_survey_system.hpp
 * @brief Manages the "Fog of War" for geological resources, tracking exploration progress over time.
 */

namespace Resources {

    /**
     * @enum SurveyStatus
     * @brief The current exploration state of a specific tile.
     */
    enum class SurveyStatus : uint8_t {
        Unknown = 0,   ///< Hidden to the player (UI renders '?')
        Surveying = 1, ///< A team is actively deployed here
        Surveyed = 2   ///< Fully revealed, rendering actual deposits
    };

    /**
     * @struct ActiveSurvey
     * @brief Tracks a deployed team's progress.
     */
    struct ActiveSurvey {
        TileCoord target;
        int32_t ticks_remaining;
    };

    /**
     * @class ResourceSurveySystem
     * @brief Maintains the global survey grid and processes active survey countdowns.
     */
    class ResourceSurveySystem {
    private:
        uint32_t width_;
        uint32_t height_;

        // A flat 1D array representing the 2D grid of survey states
        std::vector<SurveyStatus> grid_;

        // Dynamic list of currently ticking survey missions
        std::vector<ActiveSurvey> active_surveys_;

        inline size_t get_index(TileCoord coord) const {
            return static_cast<size_t>(coord.y) * width_ + static_cast<size_t>(coord.x);
        }

        inline bool is_valid(TileCoord coord) const {
            return coord.x >= 0 && coord.x < static_cast<int32_t>(width_) &&
                   coord.y >= 0 && coord.y < static_cast<int32_t>(height_);
        }

    public:
        /**
         * @brief Constructs the survey map, defaulting all tiles to Unknown.
         * @param w Map width
         * @param h Map height
         */
        ResourceSurveySystem(uint32_t w, uint32_t h) : width_(w), height_(h) {
            grid_.resize(static_cast<size_t>(w) * h, SurveyStatus::Unknown);
        }

        /**
         * @brief Deploys a team to uncover a tile's resources over a set duration.
         * @param coord The target location.
         * @param ticks_required How long the survey takes to complete.
         */
        void deploy_team(TileCoord coord, int32_t ticks_required) {
            if (!is_valid(coord)) return;

            size_t idx = get_index(coord);
            // Only allow deployment if the tile is genuinely unknown
            if (grid_[idx] == SurveyStatus::Unknown) {
                grid_[idx] = SurveyStatus::Surveying;
                active_surveys_.push_back({coord, ticks_required});
            }
        }

        /**
         * @brief Advances all active surveys by one tick. To be called by the Master Scheduler.
         */
        void tick() {
            // Process all active surveys and remove completed ones safely
            for (auto it = active_surveys_.begin(); it != active_surveys_.end(); ) {
                it->ticks_remaining--;

                if (it->ticks_remaining <= 0) {
                    grid_[get_index(it->target)] = SurveyStatus::Surveyed;

                    // Erase returns the iterator to the next element
                    it = active_surveys_.erase(it);
                } else {
                    ++it;
                }
            }
        }

        /**
         * @brief Checks the exact status of a tile (used by the UI to determine what to render).
         */
        SurveyStatus get_status(TileCoord coord) const {
            if (is_valid(coord)) {
                return grid_[get_index(coord)];
            }
            return SurveyStatus::Unknown;
        }

        /**
         * @brief Helper function to quickly check if deposits can be legally revealed.
         */
        bool is_surveyed(TileCoord coord) const {
            return get_status(coord) == SurveyStatus::Surveyed;
        }
    };
}