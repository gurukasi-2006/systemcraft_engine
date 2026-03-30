#pragma once

#include <string>
#include <cstdint>
#include <unordered_set>
#include <string_view>
#include "../data/tables/name_tables.hpp"
#include "../../../world/06_Worldgen/seed_manager.hpp"

/**
 * @file identity_component.hpp
 * @brief Immutable core identity of a simulated citizen.
 */

namespace Population {

    /**
     * @enum BiologicalSex
     * @brief Determines demographic tracking and name generation for the citizen.
     */
    enum class BiologicalSex : uint8_t {
        Male = 0,
        Female = 1
    };

    /**
     * @struct IdentityComponent
     * @brief Holds constant personal data. Cannot be modified after instantiation.
     */
    struct IdentityComponent {
        const uint32_t citizen_id;
        const std::string full_name;
        const uint64_t birth_tick;
        const BiologicalSex sex;


        IdentityComponent(uint32_t id, std::string name, uint64_t b_tick, BiologicalSex s)
            : citizen_id(id), full_name(std::move(name)), birth_tick(b_tick), sex(s) {}

        /**
         * @brief O(1) Age calculation using raw simulation ticks.
         * @param current_tick The engine's absolute time counter.
         * @return Age in simulation ticks (can be converted to years by UI later).
         */
        inline uint64_t get_age_ticks(uint64_t current_tick) const {
            return (current_tick > birth_tick) ? (current_tick - birth_tick) : 0;
        }
    };

    /**
     * @class IdentityFactory
     * @brief Generates unique identities for new citizens, enforcing name uniqueness.
     */
    class IdentityFactory {
    private:
        std::unordered_set<std::string> issued_names_;
        uint32_t next_citizen_id_ = 1;

    public:
        /**
         * @brief Spawns a guaranteed-unique identity component using the NameTables data.
         * @param current_tick The exact tick the citizen is spawned.
         * @param rng The deterministic random number generator.
         */
        IdentityComponent generate_identity(uint64_t current_tick, SeedManager& rng) {
            //  Determine Biological Sex First
            BiologicalSex sex = (rng.random_int(0, 1) == 0) ? BiologicalSex::Male : BiologicalSex::Female;

            std::string generated_name;
            bool is_unique = false;

            // Array sizes based on  name_tables.hpp
            size_t n_first = (sex == BiologicalSex::Male) ? NameTables::MALE_FIRST.size() : NameTables::FEMALE_FIRST.size();
            size_t n_surnames = NameTables::SURNAMES.size();
            size_t max_combinations = n_first * n_surnames; // 64 * 64 = 4096 unique names per sex
            size_t attempts = 0;
            size_t fallback_counter = 1;

            // Generating Unique Name
            while (!is_unique) {
                std::string_view first = (sex == BiologicalSex::Male)
                    ? NameTables::MALE_FIRST[rng.random_int(0, static_cast<int32_t>(n_first - 1))]
                    : NameTables::FEMALE_FIRST[rng.random_int(0, static_cast<int32_t>(n_first - 1))];

                std::string_view last = NameTables::SURNAMES[rng.random_int(0, static_cast<int32_t>(n_surnames - 1))];

                generated_name = std::string(first) + " " + std::string(last);

                if (issued_names_.find(generated_name) == issued_names_.end()) {
                    is_unique = true;
                } else {
                    attempts++;

                    if (attempts > max_combinations) {
                        fallback_counter++;
                        std::string fallback_name = generated_name + " " + std::to_string(fallback_counter);
                        if (issued_names_.find(fallback_name) == issued_names_.end()) {
                            generated_name = fallback_name;
                            is_unique = true;
                        }
                    }
                }
            }

            // Locking the name in the global registry
            issued_names_.insert(generated_name);

            // Assigning sequential CitizenID
            uint32_t id = next_citizen_id_++;

            return IdentityComponent(id, generated_name, current_tick, sex);
        }

        /** @brief Expose size for rapid demographic queries. */
        size_t get_population_count() const { return issued_names_.size(); }
    };
}