#pragma once

#include <cstdint>

/**
 * @file terrain_type.hpp
 * @brief Defines the classification of natural, synthetic, and event-state terrain tiles[cite: 448].
 */

/**
 * @enum TerrainType
 * @brief A strictly ordered enumeration of all 24 terrain types. Used to drive construction costs, agricultural yields, and transport modifiers[cite: 449, 450].
 * * IMPORTANT: New terrain types must ONLY be appended directly above COUNT.
 */
enum class TerrainType : uint8_t {

    /**
     * @name LOWLAND & OPEN
     * @brief Baseline terrain types with minimal geological obstruction.
     */
    ///@{

    /** * @brief Plains. The foundation of every great nation. Flat, accessible, forgettable[cite: 455, 493].
     * @details Modifiers: Const x0.9 | Agri x1.0 | Trans x1.0 | Pop x1.2 | Dep Low[cite: 499].
     */
    Plains,

    /** * @brief Grassland. The natural home of livestock and rural politics[cite: 456, 505].
     * @details Modifiers: Const x0.95 | Agri x1.2 | Trans x1.0 | Pop x0.9 | Dep Low[cite: 510].
     */
    Grassland,

    /** * @brief Forest. Renewable wealth. Environmental battleground[cite: 457, 519].
     * @details Modifiers: Const x1.3 | Agri x0.4 | Trans x0.6 | Pop x0.5 | Dep Medium[cite: 524].
     */
    Forest,

    /** * @brief Farmland. The most contested tile on the map[cite: 458, 529].
     * @details Modifiers: Const x1.1 | Agri x1.5 | Trans x1.0 | Pop x0.6 | Dep None[cite: 534].
     */
    Farmland,

    /** * @brief Wetland. Nature's water filter. An engineer's nightmare[cite: 459, 543].
     * @details Modifiers: Const x2.2 | Agri x0.7 | Trans x0.4 | Pop x0.3 | Dep None[cite: 549].
     */
    Wetland,

    /** * @brief Degraded Land. What happens when no one pays the environmental bill[cite: 460, 555].
     * @details Modifiers: Const x0.7 | Agri x0.1 | Trans x0.8 | Pop x0.4 | Dep None[cite: 561].
     */
    DegradedLand,
    ///@}

    /**
     * @name ELEVATED & ROCKY
     * @brief Terrain with significant elevation or structural challenges.
     */
    ///@{

    /** * @brief Hills. Cheap enough to cross. Expensive enough to notice[cite: 462, 570].
     * @details Modifiers: Const x1.4 | Agri x0.75 | Trans x0.75 | Pop x0.8 | Dep High[cite: 577].
     */
    Hills,

    /** * @brief Mountain. The geological wall that decides who trades with whom[cite: 463, 583].
     * @details Modifiers: Const x4.5 | Agri x0.1 | Trans x0.25 | Pop x0.2 | Dep Very High[cite: 589].
     */
    Mountain,

    /** * @brief Rocky Outcrop. Barren surface. Rich beneath[cite: 464, 598].
     * @details Modifiers: Const x2.8 | Agri x0.05 | Trans x0.55 | Pop x0.2 | Dep Very High[cite: 604].
     */
    Rocky,

    /** * @brief Volcanic. Catastrophic risk. Exceptional fertility. A bargain with geology[cite: 465, 610].
     * @details Modifiers: Const x1.6 | Agri x1.8 | Trans x0.7 | Pop x0.7 | Dep High[cite: 617].
     */
    Volcanic,
    ///@}

    /**
     * @name WATER & WETLAND
     * @brief Hydrological terrain features[cite: 466].
     */
    ///@{

    /** * @brief River. The original highway. The original border[cite: 470, 627].
     * @details Modifiers: Const x3.5 | Agri x1.3 | Trans x1.4 | Pop x1.1 | Dep All[cite: 631].
     */
    River,

    /** * @brief Lake. A strategic water reserve. A beautiful problem[cite: 471, 636].
     * @details Modifiers: Const N/A | Agri x1.4 | Trans N/A | Pop x1.2 | Dep Adjacent[cite: 643].
     */
    Lake,

    /** * @brief Ocean. The world beyond your borders[cite: 472, 652].
     * @details Modifiers: Const x6.0 | Agri N/A | Trans x1.6 | Pop N/A | Dep Offshore[cite: 659].
     */
    Ocean,
    ///@}

    /**
     * @name ARID & EXTREME
     * @brief Climatically hostile terrain types[cite: 473].
     */
    ///@{

    /** * @brief Desert. Hostile to life. Generous with oil[cite: 474, 666].
     * @details Modifiers: Const x1.5 | Agri x0.05 | Trans x0.7 | Pop x0.15 | Dep Very High[cite: 672, 674].
     */
    Desert,

    /** * @brief Semi-Arid. The frontier between productivity and dust[cite: 475, 682].
     * @details Modifiers: Const x1.2 | Agri x0.5 | Trans x0.85 | Pop x0.5 | Dep Medium[cite: 687].
     */
    SemiArid,

    /** * @brief Tundra. Cold, remote, and sitting on the future's fuel[cite: 476, 692].
     * @details Modifiers: Const x2.5 | Agri x0.15 | Trans x0.6 | Pop x0.2 | Dep High[cite: 698, 701].
     */
    Tundra,

    /** * @brief Ice Sheet. The world's largest freshwater reserve. Immovable. For now[cite: 477, 706].
     * @details Modifiers: Const N/A | Agri N/A | Trans N/A | Pop N/A | Dep None[cite: 714].
     */
    IceSheet,
    ///@}

    /**
     * @name COASTAL & TRANSITIONAL
     * @brief Terrain at the boundary of land and water[cite: 478].
     */
    ///@{

    /** * @brief Coast. Every great civilisation was built here[cite: 479, 721].
     * @details Modifiers: Const x1.1 | Agri x0.9 | Trans x1.0/x1.8 | Pop x1.3 | Dep Medium[cite: 731].
     */
    Coast,

    /** * @brief Floodplain. The richest soil. The most unreliable neighbour[cite: 480, 737].
     * @details Modifiers: Const x1.3 | Agri x0.3/x1.6 | Trans x0.7 | Pop x0.8 | Dep None[cite: 743].
     */
    Floodplain,

    /** * @brief Delta. Where rivers meet the sea. Where nations are born[cite: 481, 749].
     * @details Modifiers: Const x1.2 | Agri x1.5 | Trans x0.9 | Pop x1.7 | Dep None[cite: 757].
     */
    Delta,
    ///@}

    /**
     * @name SPECIAL & SYNTHETIC
     * @brief Anthropogenic zones or temporary event states[cite: 482].
     */
    ///@{

    /** * @brief Urban. Not a terrain. The absence of terrain[cite: 483, 767].
     * @details Modifiers: Const x1.0 | Agri x0 | Trans x0.8 | Pop x2.0 | Dep None[cite: 774].
     */
    Urban,

    /** * @brief Industrial Zone. Prosperity, pollution, and politics in one tile[cite: 484, 781].
     * @details Modifiers: Const x1.0 | Agri x0 | Trans x0.9 | Pop x0 | Dep None[cite: 790].
     */
    IndustrialZone,

    /** * @brief Reclaimed Land. Land the sea gave up. For a price[cite: 486, 797].
     * @details Modifiers: Const x3.5 | Agri x0.8 | Trans x0.95 | Pop x1.0 | Dep None[cite: 803].
     */
    Reclaimed,

    /** * @brief Ash Fall. The aftermath of geological violence[cite: 487, 810].
     * @details Modifiers: Const x3.0 | Agri x0.05 | Trans x0.2 | Pop x0.1 | Dep None[cite: 818].
     */
    AshFall,
    ///@}

    /** * @brief The automatic array scaler. Automatically evaluates to 24[cite: 824].
     * Defines the maximum size of any terrain-based memory array.
     */
    COUNT
};