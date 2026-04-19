#pragma once

#include <entt/entt.hpp>
#include <array>
#include <algorithm>

#include "../../../core/01_ECS_core/ecs_world.hpp"
#include "../08_Citizencomponent/health_component.hpp"
#include "../10_NeedsSystem/need_definitions.hpp" // Subsystem 115

/**
 * @file need_decay_engine.hpp
 * @brief Subsystem 116: The constant thermodynamic pressure of the simulation.
 * @details Decays citizen needs per tick. Scales with physical health and global difficulty.
 */

namespace Population {

    /**
     * @enum GameDifficulty
     * @brief The global difficulty axis. Modifies the urgency of the simulation, not the raw stats.
     */
    enum class GameDifficulty {
        Sandbox, ///< 50% decay rate (Contemplative)
        Normal,  ///< 100% decay rate (Balanced)
        Hard     ///< 130% decay rate (Urgent Crisis)
    };

    class NeedDecayEngine {
    private:
        // The mathematical constants provided in the design document, mapped to the 13-tier enum.
        // Array index matches the NeedType integer value.
        static constexpr std::array<float, static_cast<size_t>(NeedType::COUNT)> BASE_DECAY_RATES = {
            0.014f,   // 0: Food
            0.020f,   // 1: Water (Fastest)
            0.002f,   // 2: Shelter
            0.001f,   // 3: Healthcare
            0.003f,   // 4: Safety
            0.005f,   // 5: Employment
            0.008f,   // 6: Transport
            0.0005f,  // 7: Education (Slowest - Generational)
            0.005f,   // 8: Leisure
            // --- The 4 Unique Systemcraft Needs ---
            0.007f,   // 9: Community (~14 days to loneliness crisis)
            0.004f,   // 10: Environment (~25 days to pollution depression)
            0.0015f,  // 11: ConsumerGoods (~45 days to material frustration)
            0.010f    // 12: Connectivity (~10 days to internet/telecom isolation)
        };

        static constexpr float get_difficulty_multiplier(GameDifficulty difficulty) {
            switch(difficulty) {
                case GameDifficulty::Sandbox: return 0.5f;
                case GameDifficulty::Hard: return 1.3f;
                case GameDifficulty::Normal:
                default: return 1.0f;
            }
        }

    public:
        /**
         * @brief Executes the thermodynamic decay of all citizen needs.
         * @param world The ECS master world.
         * @param difficulty The current global difficulty setting.
         */
        void update(ECSWorld& world, GameDifficulty difficulty = GameDifficulty::Normal) {
            float diff_mult = get_difficulty_multiplier(difficulty);

            // We query Needs, but Health is optional. If missing, we assume 100.0 (perfect health).
            auto view = world.registry.view<NeedsComponent>();

            for (auto raw_id : view) {
                auto& needs = view.get<NeedsComponent>(raw_id);

                // Fetch health, defaulting to 100.0f
                float health = 100.0f;
                if (world.registry.all_of<HealthComponent>(raw_id)) {
                    health = world.registry.get<HealthComponent>(raw_id).physical_health;
                }

                // The Compound Deprivation Spiral!
                // Health 100 = 1.0x decay. Health 30 = 1.7x decay.
                float effective_health_mod = 2.0f - (health / 100.0f);
                float final_multiplier = effective_health_mod * diff_mult;

                // Apply decay to all 13 needs in an ultra-fast, unrolled or auto-vectorized loop
                for (size_t i = 0; i < static_cast<size_t>(NeedType::COUNT); ++i) {
                    float total_decay = BASE_DECAY_RATES[i] * final_multiplier;

                    // add_to_need safely clamps it at 0.0f so it never goes negative
                    needs.add_to_need(static_cast<NeedType>(i), -total_decay);
                }
            }
        }
    };
}