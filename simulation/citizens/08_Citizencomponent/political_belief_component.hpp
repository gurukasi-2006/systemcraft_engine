#pragma once

#include <algorithm>
#include <cstdint>

/**
 * @file political_belief_component.hpp
 * @brief Subsystem 97: Tracks emergent political ideologies driven by economic stress and social contagion.
 */

namespace Population {

    /**
     * @struct PoliticalBeliefComponent
     * @brief ECS Component holding the citizen's ideological alignment.
     */
    struct PoliticalBeliefComponent {
        // Axes:
        // Economic: -1.0 (Far Left) to +1.0 (Far Right)
        // Social:   -1.0 (Libertarian) to +1.0 (Authoritarian)
        float economic_axis{0.0f};
        float social_axis{0.0f};

        /**
         * @brief Updates the citizen's political leaning based on material conditions and peer influence.
         * @param income The citizen's current monthly gross income.
         * @param median_income The current national/regional median income.
         * @param happiness The citizen's overall happiness score (0.0 to 100.0).
         * @param peer_mean_econ The average economic axis of the citizen's social network.
         * @param social_weight Multiplier for how easily influenced they are by peers (e.g., 1.0).
         * @param information_access The citizen's access to media/internet (0.0 to 100.0).
         * @param media_bias_factor Policy-driven multiplier ranging from 0.5 to 1.5.
         */
        inline void update(float income, float median_income, float happiness,
                           float peer_mean_econ, float social_weight,
                           float information_access, float media_bias_factor)
        {
            // 1. Calculate Economic Stress (1.0 = Max Stress, -1.0 = Extremely Prosperous)
            float income_ratio = 0.0f;
            if (median_income > 0.0f) {
                income_ratio = std::clamp(income / median_income, 0.0f, 2.0f);
            }
            float economic_stress = 1.0f - income_ratio;

            // 2. Base Ideological Drift from material conditions
            // Note: As per the formula, high stress pushes the axis positive, high happiness pushes negative.
            float axis_drift = (economic_stress * 0.004f) - ((happiness / 100.0f) * 0.002f);

            // 3. Media Exposure Modifier
            if (information_access > 60.0f) {
                axis_drift *= media_bias_factor;
            }

            // Apply the internal material drift
            economic_axis += axis_drift;

            // 4. Social Contagion (Interpolation toward peer mean)
            economic_axis += (peer_mean_econ - economic_axis) * 0.001f * social_weight;

            // 5. Enforce rigid ideological boundaries
            economic_axis = std::clamp(economic_axis, -1.0f, 1.0f);
            social_axis = std::clamp(social_axis, -1.0f, 1.0f);
        }
    };
}