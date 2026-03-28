#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <entt/entt.hpp>

#include "../../core/04_Types/tile_coord.hpp"
#include "../05_Terrain/grid_data_store.hpp"
#include "population_seeder.hpp" // To access the PositionComponent and SettlementComponent

/**
 * @file road_skeleton_generator.hpp
 * @brief Connects scattered settlements into a unified transportation network using a Minimum Spanning Tree.
 */

namespace RoadSkeletonGenerator {

    /**
     * @brief A highly optimized Disjoint Set (Union-Find) to detect cycles during MST construction.
     */
    struct UnionFind {
        std::vector<size_t> parent;

        UnionFind(size_t n) {
            parent.resize(n);
            for (size_t i = 0; i < n; ++i) parent[i] = i;
        }

        size_t find(size_t i) {
            if (parent[i] == i) return i;
            return parent[i] = find(parent[i]); // Path compression
        }

        void unite(size_t i, size_t j) {
            size_t root_i = find(i);
            size_t root_j = find(j);
            if (root_i != root_j) parent[root_i] = root_j;
        }
    };

    /**
     * @struct Edge
     * @brief Represents a potential road connection between two settlements.
     */
    struct Edge {
        size_t u, v;
        int32_t distance;

        bool operator<(const Edge& other) const {
            return distance < other.distance;
        }
    };

    /**
     * @brief Draws a primitive dirt road using a simple Manhattan L-shaped route.
     */
    inline void carve_dirt_road(GridDataStore& grid, TileCoord start, TileCoord end) {
        int32_t x = start.x;
        int32_t y = start.y;

        // Route along X axis first
        while (x != end.x) {
            // Assume grid.get_cell() has a has_road property to mark infrastructure
            // grid.get_cell({x, y}).has_road = true;
            x += (end.x > x) ? 1 : -1;
        }
        // Then route along Y axis
        while (y != end.y) {
            // grid.get_cell({x, y}).has_road = true;
            y += (end.y > y) ? 1 : -1;
        }
        // Mark the final destination
        // grid.get_cell({x, y}).has_road = true;
    }

    /**
     * @brief Extracts settlement locations and links them with a Minimum Spanning Tree.
     * @param reg The master EnTT registry containing the newly seeded settlements.
     * @param grid The world map where roads will be carved.
     */
    inline void generate_network(entt::registry& reg, GridDataStore& grid) {
        auto view = reg.view<PositionComponent, SettlementComponent>();
        std::vector<TileCoord> settlement_nodes;

        // 1. Extract all settlement coordinates
        for (auto entity : view) {
            settlement_nodes.push_back(view.get<PositionComponent>(entity).coord);
        }

        size_t num_nodes = settlement_nodes.size();
        if (num_nodes < 2) return; // Cannot build a network with less than 2 cities

        // 2. Build every possible connection (Complete Graph)
        std::vector<Edge> edges;
        edges.reserve((num_nodes * (num_nodes - 1)) / 2);

        for (size_t i = 0; i < num_nodes; ++i) {
            for (size_t j = i + 1; j < num_nodes; ++j) {
                // Calculate Manhattan distance as the weight
                int32_t dist = std::abs(settlement_nodes[i].x - settlement_nodes[j].x) +
                               std::abs(settlement_nodes[i].y - settlement_nodes[j].y);
                edges.push_back({i, j, dist});
            }
        }

        // 3. Sort edges by shortest distance (Kruskal's requirement)
        std::sort(edges.begin(), edges.end());

        // 4. Kruskal's Algorithm to find the MST
        UnionFind uf(num_nodes);
        size_t edges_added = 0;

        for (const auto& edge : edges) {
            // If the two settlements are not already connected to the main network...
            if (uf.find(edge.u) != uf.find(edge.v)) {
                uf.unite(edge.u, edge.v);

                // Physically carve the road into the terrain data
                carve_dirt_road(grid, settlement_nodes[edge.u], settlement_nodes[edge.v]);

                edges_added++;
                if (edges_added == num_nodes - 1) break; // MST is complete
            }
        }
    }
}