#pragma once

#include <cstdint>

/**
 * @file resource_type.hpp
 * @brief Defines the complete taxonomy of materials, goods, and emergent properties within the Systemcraft economy.
 */

/**
 * @enum ResourceType
 * @brief A strictly ordered enumeration of all 41 resources. Used globally as array indices for zero-latency logistics and economy lookups.
 * * IMPORTANT: To maintain save-file compatibility, new resources must ONLY be appended to the end of the list, directly above COUNT. CITE Systemcraft_Resource_Design.pdf for more info
 */
enum class ResourceType : uint16_t {

    /**
     * @name RAW EXTRACTION
     * @brief Foundational resources extracted directly from the grid.
     */
    ///@{

    /** @brief Tier 0. The backbone of every industrial nation. */
    IronOre,

    /** @brief Tier 0. Dirty, essential, politically toxic. */
    Coal,

    /** @brief Tier 0. Liquid power. Liquid dependency. */
    CrudeOil,

    /** @brief Tier 0. Cleaner than coal. More fragile than oil. */
    NaturalGas,

    /** @brief Tier 0. The wiring of civilisation. */
    CopperOre,

    /** @brief Tier 0. The ore behind everything lightweight. */
    Bauxite,

    /** @brief Tier 0. Enormous power. Enormous responsibility. */
    Uranium,

    /** @brief Tier 0. The hidden gatekeeper of the modern economy. */
    RareEarthElements,

    /** @brief Tier 0. The resource that stores the future. */
    Lithium,

    /** @brief Tier 0. Unglamorous. Indispensable. */
    Limestone,

    /** @brief Tier 0. The invisible multiplier of food security. */
    Potash,
    ///@}

    /**
     * @name AGRICULTURE
     * @brief Organic resources produced by farms and forestry.
     */
    ///@{

    /** @brief Tier 0. The floor beneath every citizen's feet. */
    Wheat,

    /** @brief Tier 0. Food, feed, and fuel all in one. */
    Maize,

    /** @brief Tier 0-1. Protein, wool, and political complexity. */
    Livestock,

    /** @brief Tier 0. Renewable if you are patient. Gone if you are not. */
    Timber,

    /** @brief Tier 0. The thread connecting workers to their living standards. */
    Cotton,

    /** @brief Tier 0. Without it, nothing moves. */
    Rubber,

    /** @brief Tier 0. The resource that funds governments and poisons citizens. */
    Tobacco,

    /** @brief Tier 0. Nature's pharmacy. The foundation of public health. */
    MedicinalPlants,
    ///@}

    /**
     * @name PROCESSED MATERIALS
     * @brief Tier 1 industrial materials processed from raw extraction.
     */
    ///@{

    /** @brief Tier 1. The material everything else is built from. */
    Steel,

    /** @brief Tier 1. Light, strong, and expensive to make. */
    Aluminium,

    /** @brief Tier 1. The blood of the transport network. */
    RefinedFuel,

    /** @brief Tier 1. Without it, nothing gets built. */
    Cement,

    /** @brief Tier 1. The inputs that make other inputs. */
    Chemicals,

    /** @brief Tier 1. The gatekeeper of the modern economy. */
    ElectronicsComponents,

    /** @brief Tier 1. The yield multiplier . */
    Fertilizer,
    ///@}

    /**
     * @name FINISHED GOODS
     * @brief Complex multi-input goods required for advanced simulation.
     */
    ///@{

    /** @brief Tier 2. The force multiplier of industrial output. */
    Machinery,

    /** @brief Tier 2. Every citizen needs one. Every road carries them. */
    Vehicles,

    /** @brief Tier 2. The measure of whether ordinary life is getting better. */
    ConsumerGoods,

    /** @brief Tier 2. The resource that determines how long citizens live. */
    Pharmaceuticals,

    /** @brief Tier 2. The industrial output that keeps the peace or breaks it. */
    WeaponsAndMunitions,
    ///@}

    /**
     * @name FOOD PRODUCTS
     * @brief Processed sustenance satisfying citizen needs.
     */
    ///@{

    /** @brief Tier 1. The baseline of civilisation. */
    FlourAndBread,

    /** @brief Tier 1. Protein for the masses. Profit for the farms. */
    MeatAndDairy,

    /** @brief Tier 1. The resource governments love to tax and citizens love to buy. */
    Alcohol,

    /** @brief Tier 1. The resource that outlasts disaster. */
    PreservedAndCannedFood,
    ///@}

    /**
     * @name ENERGY CARRIERS
     * @brief Real-time generation grids and advanced fuels.
     */
    ///@{

    /** @brief Tier 1. The resource you cannot stockpile. The resource you cannot lack. */
    Electricity,

    /** @brief Tier 2. The clean energy carrier of the mature nation. */
    HydrogenFuel,
    ///@}

    /**
     * @name SPECIAL & EMERGENT
     * @brief Abstract, non-physical, or dynamic byproduct resources.
     */
    ///@{

    /** @brief Emergent. The market the government created by trying to stop the market. */
    Contraband,

    /** @brief Emergent. The price of production nobody wants to pay. */
    IndustrialWaste,

    /** @brief Emergent. The resource you grow, not dig up. */
    SkilledLabor,

    /** @brief Tier 3. The resource that makes every other resource more efficient. */
    DataAndInformation,
    ///@}

    /** * @brief The automatic array scaler. Automatically evaluates to 41.
     * Defines the maximum size of any resource-based memory array.
     */
    COUNT
};