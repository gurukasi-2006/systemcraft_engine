#pragma once

#include <cstdint>

/**
 * @file terrain_type.hpp
 * @brief Defines the classification of natural, synthetic, and event-state terrain tiles.
 */

/**
 * @enum TerrainType
 * @brief A strictly ordered enumeration of all 24 terrain types. Used to drive construction costs, agricultural yields, and transport modifiers.
 * * IMPORTANT: New terrain types must ONLY be appended directly above COUNT.
 */
enum class TerrainType : uint8_t {

    /**
     * @name LOWLAND & OPEN
     * @brief Baseline terrain types with minimal geological obstruction.
     */
    ///@{

    /** * @brief Plains. The foundation of every great nation. Flat, accessible, forgettable.
     * @details Modifiers: Const x0.9 | Agri x1.0 | Trans x1.0 | Pop x1.2 | Dep Low.
     */
    Plains,

    /** * @brief Grassland. The natural home of livestock and rural politics.
     * @details Modifiers: Const x0.95 | Agri x1.2 | Trans x1.0 | Pop x0.9 | Dep Low.
     */
    Grassland,

    /** * @brief Forest. Renewable wealth. Environmental battleground.
     * @details Modifiers: Const x1.3 | Agri x0.4 | Trans x0.6 | Pop x0.5 | Dep Medium.
     */
    Forest,

    /** * @brief Farmland. The most contested tile on the map.
     * @details Modifiers: Const x1.1 | Agri x1.5 | Trans x1.0 | Pop x0.6 | Dep None.
     */
    Farmland,

    /** * @brief Wetland. Nature's water filter. An engineer's nightmare.
     * @details Modifiers: Const x2.2 | Agri x0.7 | Trans x0.4 | Pop x0.3 | Dep None.
     */
    Wetland,

    /** * @brief Degraded Land. What happens when no one pays the environmental bill.
     * @details Modifiers: Const x0.7 | Agri x0.1 | Trans x0.8 | Pop x0.4 | Dep None.
     */
    DegradedLand,
    ///@}

    /**
     * @name ELEVATED & ROCKY
     * @brief Terrain with significant elevation or structural challenges.
     */
    ///@{

    /** * @brief Hills. Cheap enough to cross. Expensive enough to notice.
     * @details Modifiers: Const x1.4 | Agri x0.75 | Trans x0.75 | Pop x0.8 | Dep High.
     */
    Hills,

    /** * @brief Mountain. The geological wall that decides who trades with whom.
     * @details Modifiers: Const x4.5 | Agri x0.1 | Trans x0.25 | Pop x0.2 | Dep Very High.
     */
    Mountain,

    /** * @brief Rocky Outcrop. Barren surface. Rich beneath.
     * @details Modifiers: Const x2.8 | Agri x0.05 | Trans x0.55 | Pop x0.2 | Dep Very High.
     */
    Rocky,

    /** * @brief Volcanic. Catastrophic risk. Exceptional fertility. A bargain with geology.
     * @details Modifiers: Const x1.6 | Agri x1.8 | Trans x0.7 | Pop x0.7 | Dep High.
     */
    Volcanic,
    ///@}

    /**
     * @name WATER & WETLAND
     * @brief Hydrological terrain features.
     */
    ///@{

    /** * @brief River. The original highway. The original border.
     * @details Modifiers: Const x3.5 | Agri x1.3 | Trans x1.4 | Pop x1.1 | Dep All.
     */
    River,

    /** * @brief Lake. A strategic water reserve. A beautiful problem.
     * @details Modifiers: Const N/A | Agri x1.4 | Trans N/A | Pop x1.2 | Dep Adjacent.
     */
    Lake,

    /** * @brief Ocean. The world beyond your borders.
     * @details Modifiers: Const x6.0 | Agri N/A | Trans x1.6 | Pop N/A | Dep Offshore.
     */
    Ocean,
    ///@}

    /**
     * @name ARID & EXTREME
     * @brief Climatically hostile terrain types.
     */
    ///@{

    /** * @brief Desert. Hostile to life. Generous with oil.
     * @details Modifiers: Const x1.5 | Agri x0.05 | Trans x0.7 | Pop x0.15 | Dep Very High.
     */
    Desert,

    /** * @brief Semi-Arid. The frontier between productivity and dust.
     * @details Modifiers: Const x1.2 | Agri x0.5 | Trans x0.85 | Pop x0.5 | Dep Medium.
     */
    SemiArid,

    /** * @brief Tundra. Cold, remote, and sitting on the future's fuel.
     * @details Modifiers: Const x2.5 | Agri x0.15 | Trans x0.6 | Pop x0.2 | Dep High.
     */
    Tundra,

    /** * @brief Ice Sheet. The world's largest freshwater reserve. Immovable. For now.
     * @details Modifiers: Const N/A | Agri N/A | Trans N/A | Pop N/A | Dep None.
     */
    IceSheet,
    ///@}

    /**
     * @name COASTAL & TRANSITIONAL
     * @brief Terrain at the boundary of land and water.
     */
    ///@{

    /** * @brief Coast. Every great civilisation was built here.
     * @details Modifiers: Const x1.1 | Agri x0.9 | Trans x1.0/x1.8 | Pop x1.3 | Dep Medium.
     */
    Coast,

    /** * @brief Floodplain. The richest soil. The most unreliable neighbour.
     * @details Modifiers: Const x1.3 | Agri x0.3/x1.6 | Trans x0.7 | Pop x0.8 | Dep None.
     */
    Floodplain,

    /** * @brief Delta. Where rivers meet the sea. Where nations are born.
     * @details Modifiers: Const x1.2 | Agri x1.5 | Trans x0.9 | Pop x1.7 | Dep None.
     */
    Delta,
    ///@}

    /**
     * @name SPECIAL & SYNTHETIC
     * @brief Anthropogenic zones or temporary event states.
     */
    ///@{

    /** * @brief Urban. Not a terrain. The absence of terrain.
     * @details Modifiers: Const x1.0 | Agri x0 | Trans x0.8 | Pop x2.0 | Dep None.
     */
    Urban,

    /** * @brief Industrial Zone. Prosperity, pollution, and politics in one tile.
     * @details Modifiers: Const x1.0 | Agri x0 | Trans x0.9 | Pop x0 | Dep None.
     */
    IndustrialZone,

    /** * @brief Reclaimed Land. Land the sea gave up. For a price.
     * @details Modifiers: Const x3.5 | Agri x0.8 | Trans x0.95 | Pop x1.0 | Dep None.
     */
    Reclaimed,

    /** * @brief Ash Fall. The aftermath of geological violence.
     * @details Modifiers: Const x3.0 | Agri x0.05 | Trans x0.2 | Pop x0.1 | Dep None.
     */
    AshFall,
    ///@}

    /** * @brief The automatic array scaler. Automatically evaluates to 24.
     * Defines the maximum size of any terrain-based memory array.
     */
    COUNT
};