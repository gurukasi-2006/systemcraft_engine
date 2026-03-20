#pragma once

#include <vector>
#include <algorithm>

/**
 * @file priority_sorter.hpp
 * @brief Dedicated subsystem for sorting engine tasks by numeric priority.
 */

class PrioritySorter {
public:
    /**
     * @brief Sorts a vector of priority-paired items in ascending order.
     * @tparam T The type of the item being sorted (e.g., unique pointer to a system).
     * @param items The vector to sort. Modifies it directly in memory.
     */
    template<typename T>
    static void sort(std::vector<std::pair<int, T>>& items) {
        std::sort(items.begin(), items.end(), [](const auto& a, const auto& b) {
            return a.first < b.first; // Lower numbers float to the front
        });
    }
};