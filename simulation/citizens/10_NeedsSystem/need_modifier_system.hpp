#pragma once

#include <unordered_map>
#include <vector>
#include <array>

#include "../../../core/03_Event_Bus/subscriber_registry.hpp"
#include "need_definitions.hpp"

/**
 * @file need_modifier_system.hpp
 * @brief Subsystem 126: Applies synergistic multiplicative buffs/debuffs to need decay rates via policies.
 */

namespace Population {

    /**
     * @struct PolicyEffect
     * @brief A single modifier applied to a specific need.
     */
    struct PolicyEffect {
        NeedType need;
        float decay_multiplier; // e.g., 0.6 = 40% slower decay
    };

    struct PolicyActivatedEvent {
        uint32_t policy_id;
        std::vector<PolicyEffect> effects;
    };

    struct PolicyDeactivatedEvent {
        uint32_t policy_id;
    };

    class NeedModifierSystem {
    private:
        // Holds the raw active policy data
        std::unordered_map<uint32_t, std::vector<PolicyEffect>> active_policies_;

        // Caches the final multiplicative product for ultra-fast O(1) lookups during the tick loop
        std::array<float, static_cast<size_t>(NeedType::COUNT)> global_multipliers_;

        /**
         * @brief Recomputes the 13-element cache. Only runs when a policy is activated/deactivated!
         */
        void recalculate_multipliers() {
            // Reset all to 1.0 (Neutral baseline)
            global_multipliers_.fill(1.0f);

            for (const auto& [policy_id, effects] : active_policies_) {
                for (const auto& effect : effects) {
                    // Multiplicative stacking!
                    global_multipliers_[static_cast<size_t>(effect.need)] *= effect.decay_multiplier;
                }
            }
        }

    public:
        /**
         * @brief Links the modifier system to the central event bus.
         * @param registry The event subscriber registry.
         */
        NeedModifierSystem(SubscriberRegistry& registry) {
            global_multipliers_.fill(1.0f);

            // Bind to Policy Activated
            registry.subscribe<PolicyActivatedEvent>([this](const PolicyActivatedEvent& ev) {
                active_policies_[ev.policy_id] = ev.effects;
                recalculate_multipliers();
            });

            // Bind to Policy Deactivated
            registry.subscribe<PolicyDeactivatedEvent>([this](const PolicyDeactivatedEvent& ev) {
                active_policies_.erase(ev.policy_id);
                recalculate_multipliers();
            });
        }

        /**
         * @brief Extremely fast O(1) getter used by the NeedDecayEngine.
         * @param type The specific need being decayed.
         * @return The pre-calculated synergistic multiplier.
         */
        inline float get_multiplier(NeedType type) const {
            return global_multipliers_[static_cast<size_t>(type)];
        }
    };
}