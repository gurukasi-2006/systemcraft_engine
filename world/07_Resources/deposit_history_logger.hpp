#pragma once

#include <unordered_map>
#include <vector>
#include <cstdint>

#include "../../core/04_Types/tile_coord.hpp"
#include "../../core/04_Types/resource_type.hpp"
#include "../../core/04_Types/fixed_point.hpp"
#include "../../core/04_Types/entity_id.hpp"

/**
 * @file deposit_history_logger.hpp
 * @brief Records the lifetime statistics of all resource deposits for the Stats Bureau and end-game graphs.
 */

namespace Resources {

    /**
     * @struct DepositHistoricalRecord
     * @brief A complete timeline and statistical summary of a single resource deposit.
     */
    struct DepositHistoricalRecord {
        EntityID original_entity;
        TileCoord location;
        ResourceType type;

        Fixed32 peak_reserves{0.0f};
        Fixed32 total_extracted{0.0f};

        uint32_t discovery_tick{0};
        uint32_t depletion_tick{0};

        bool is_depleted{false};
    };

    /**
     * @class DepositHistoryLogger
     * @brief Tracks deposits from discovery to exhaustion, archiving dead veins for historical rendering.
     */
    class DepositHistoryLogger {
    private:
        // Fast lookup for currently active mines
        std::unordered_map<uint32_t, DepositHistoricalRecord> active_records_;

        // Contiguous memory for dead/exhausted veins (Great for UI rendering/graphing)
        std::vector<DepositHistoricalRecord> historical_archive_;

    public:
        /**
         * @brief Logs the discovery of a brand new deposit.
         */
        void log_discovery(EntityID id, TileCoord loc, ResourceType type, Fixed32 initial_reserves, uint32_t current_tick) {
            DepositHistoricalRecord record;
            record.original_entity = id;
            record.location = loc;
            record.type = type;
            record.peak_reserves = initial_reserves;
            record.total_extracted = Fixed32(0.0f);
            record.discovery_tick = current_tick;
            record.is_depleted = false;

            active_records_[id.raw_id] = record;
        }

        /**
         * @brief Records an extraction event, accumulating the lifetime output of the vein.
         */
        void log_extraction(EntityID id, Fixed32 amount_extracted) {
            auto it = active_records_.find(id.raw_id);
            if (it != active_records_.end()) {
                it->second.total_extracted = it->second.total_extracted + amount_extracted;
            }
        }

        /**
         * @brief Updates the peak reserves (Useful for renewable resources like Timber that might grow).
         */
        void log_peak_update(EntityID id, Fixed32 new_reserve_level) {
            auto it = active_records_.find(id.raw_id);
            if (it != active_records_.end()) {
                if (new_reserve_level > it->second.peak_reserves) {
                    it->second.peak_reserves = new_reserve_level;
                }
            }
        }

        /**
         * @brief Marks a deposit as exhausted, stamping the date and moving it to the permanent archive.
         */
        void log_depletion(EntityID id, uint32_t current_tick) {
            auto it = active_records_.find(id.raw_id);
            if (it != active_records_.end()) {
                it->second.is_depleted = true;
                it->second.depletion_tick = current_tick;

                // Move to the permanent historical archive
                historical_archive_.push_back(it->second);

                // Remove from the active tracking map to save memory/lookup time
                active_records_.erase(it);
            }
        }

        /**
         * @brief Retrieves the permanent archive for UI rendering or Stats Bureau analysis.
         */
        const std::vector<DepositHistoricalRecord>& get_archive() const {
            return historical_archive_;
        }

        /**
         * @brief Retrieves an active record if it still exists.
         */
        bool get_active_record(EntityID id, DepositHistoricalRecord& out_record) const {
            auto it = active_records_.find(id.raw_id);
            if (it != active_records_.end()) {
                out_record = it->second;
                return true;
            }
            return false;
        }
    };
}