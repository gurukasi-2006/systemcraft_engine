#pragma once

#include <unordered_map>
#include <cstdint>

#include "../../core/04_Types/resource_type.hpp"
#include "../../core/04_Types/fixed_point.hpp"

/**
 * @file import_export_resource_ledger.hpp
 * @brief Tracks national extraction, imports, and exports per resource type for GDP calculations.
 */

namespace Resources {

    /**
     * @struct ResourceAnnualStats
     * @brief Holds the macroeconomic totals for a single resource type over a given period (e.g., one year).
     */
    struct ResourceAnnualStats {
        Fixed32 total_extracted{0.0f};
        Fixed32 total_imported{0.0f};
        Fixed32 total_exported{0.0f};

        /**
         * @brief Calculates the net domestic availability of this resource.
         * @return (Extracted + Imported) - Exported
         */
        Fixed32 get_net_domestic_supply() const {
            return (total_extracted + total_imported) - total_exported;
        }
    };

    /**
     * @class ImportExportResourceLedger
     * @brief A centralized financial ledger used by the Economy System and Stats Bureau.
     */
    class ImportExportResourceLedger {
    private:
        // Maps each specific resource type to its macroeconomic statistics
        std::unordered_map<ResourceType, ResourceAnnualStats> ledger_;

    public:
        /**
         * @brief Records newly extracted raw materials (e.g., called by the ReserveDepletionTracker).
         * @param type The resource being mined/harvested.
         * @param amount The yield generated this tick.
         */
        void record_extraction(ResourceType type, Fixed32 amount) {
            if (amount > Fixed32(0.0f)) {
                ledger_[type].total_extracted = ledger_[type].total_extracted + amount;
            }
        }

        /**
         * @brief Records resources purchased from foreign nations or AI factions.
         */
        void record_import(ResourceType type, Fixed32 amount) {
            if (amount > Fixed32(0.0f)) {
                ledger_[type].total_imported = ledger_[type].total_imported + amount;
            }
        }

        /**
         * @brief Records resources sold to foreign nations or AI factions.
         */
        void record_export(ResourceType type, Fixed32 amount) {
            if (amount > Fixed32(0.0f)) {
                ledger_[type].total_exported = ledger_[type].total_exported + amount;
            }
        }

        /**
         * @brief Retrieves the full statistical block for a specific resource.
         */
        ResourceAnnualStats get_stats(ResourceType type) const {
            auto it = ledger_.find(type);
            if (it != ledger_.end()) {
                return it->second;
            }
            return ResourceAnnualStats{}; // Returns zeroes if nothing has been tracked yet
        }

        /**
         * @brief Resets all statistics to zero. To be called by the CalendarSystem on New Year's Day.
         */
        void reset_annual_stats() {
            ledger_.clear();
        }
    };
}