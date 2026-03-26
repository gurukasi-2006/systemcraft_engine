#pragma once

/**
 * @file economic_constants.hpp
 * @brief Defines the compile-time economic constants, elasticities, and dampening factors. [cite: 834]
 * @details Contains 48 constants divided across 12 domain namespaces. Override via GlobalConfig at runtime for scenarios. [cite: 837, 1679]
 */

/**
 * @namespace Tax
 * @brief All taxation: income, corporate, VAT, property, royalties, excise. [cite: 836]
 */
namespace Tax {
    /** @brief The contract between citizen and state. [cite: 845]
     * @details Fraction of gross income. [cite: 1659] */
    constexpr auto BASE_INCOME_TAX_RATE = 0.20f;

    /** @brief The price of doing business in your nation. [cite: 846]
     * @details Fraction of industry net profit. [cite: 1659] */
    constexpr auto BASE_CORPORATE_TAX_RATE = 0.25f;

    /** @brief The invisible tax. Regressive by design. [cite: 847]
     * @details Fraction added to consumer goods price. [cite: 1660] */
    constexpr auto BASE_VAT_RATE = 0.15f;

    /** @brief The tax that shapes cities. [cite: 848]
     * @details Fraction of building market valuation per year. [cite: 1660] */
    constexpr auto BASE_PROPERTY_TAX_RATE = 0.01f;

    /** @brief The sovereign's share of the ground beneath. [cite: 849]
     * @details Fraction of gross extraction value. [cite: 1661] */
    constexpr auto RESOURCE_EXTRACTION_ROYALTY = 0.08f;

    /** @brief Zero by default. The cost of ignoring it is measured in terrain. [cite: 850]
     * @details Currency units per tonne CO2-equivalent per tick. [cite: 1661] */
    constexpr auto CARBON_TAX_PER_TONNE = 0.0f;

    /** @brief The government's most reliable sin revenue. [cite: 851]
     * @details Fraction of retail price as excise duty. [cite: 1662] */
    constexpr auto TOBACCO_EXCISE_RATE = 0.55f;

    /** @brief Enough to collect. Not enough to ban. [cite: 852]
     * @details Fraction of retail price as excise duty. [cite: 1662] */
    constexpr auto ALCOHOL_EXCISE_RATE = 0.35f;
}

/**
 * @namespace Market
 * @brief Price elasticities, bubble detection, subsidy pass-through. [cite: 836]
 */
namespace Market {
    /** @brief People will pay almost anything not to starve. [cite: 855]
     * @details % demand change per % price change. [cite: 1663] */
    constexpr auto PRICE_ELASTICITY_FOOD = -0.30f;

    /** @brief Expensive energy hurts everything, slowly. [cite: 855]
     * @details % demand change per % price change. [cite: 1663] */
    constexpr auto PRICE_ELASTICITY_ENERGY = -0.45f;

    /** @brief Luxuries are optional. Citizens know this. [cite: 856]
     * @details % demand change per % price change. [cite: 1663] */
    constexpr auto PRICE_ELASTICITY_CONSUMER_GOODS = -1.20f;

    /** @brief Shelter is not optional. Neither is political backlash. [cite: 856, 857]
     * @details % demand change per % rent increase. [cite: 1664] */
    constexpr auto PRICE_ELASTICITY_HOUSING_RENT = -0.20f;

    /** @brief Eight percent a month. The market is lying to you. [cite: 857]
     * @details Monthly price growth rate fraction above which bubble warning fires. [cite: 1664] */
    constexpr auto BUBBLE_DETECTION_GROWTH_THRESHOLD = 0.08f;

    /** @brief Markets fear uncertainty more than bad news. [cite: 858]
     * @details Weight of price volatility in confidence index (0-1). [cite: 1664, 1665] */
    constexpr auto MARKET_CONFIDENCE_VOLATILITY_WEIGHT = 0.60f;

    /** @brief Half of every subsidy disappears before it helps anyone. [cite: 859]
     * @details Fraction of subsidy that reaches consumer as lower price. [cite: 1665] */
    constexpr auto SUBSIDY_CONSUMER_PASSTHROUGH_RATE = 0.55f;
}

/**
 * @namespace Monetary
 * @brief Inflation targeting, interest rates, wage-price spiral, dampening. [cite: 836]
 */
namespace Monetary {
    /** @brief Two percent. The most consequential number in economics. [cite: 861]
     * @details Annual fraction (monthly: 0.00167f). [cite: 1665] */
    constexpr auto TARGET_INFLATION_RATE = 0.02f;

    /** @brief The central bank's patience, measured precisely. [cite: 862]
     * @details Fraction either side of target before central bank acts. [cite: 1666] */
    constexpr auto INFLATION_TOLERANCE_BAND = 0.01f;

    /** @brief The economy self-corrects. Slowly. Painfully. [cite: 862]
     * @details Fraction by which excess inflation reduces itself per month. [cite: 1666] */
    constexpr auto INFLATION_DAMPENING_FACTOR = 0.15f;

    /** @brief Workers ask for more. Firms charge more. Repeat. [cite: 863]
     * @details Fraction of CPI inflation that feeds into wage demands each month. [cite: 1666, 1667] */
    constexpr auto WAGE_PRICE_SPIRAL_COEFFICIENT = 0.40f;

    /** @brief The Taylor Rule, simplified for a nation-builder. [cite: 863]
     * @details Basis points interest rate rise per 1% inflation above target. [cite: 1667] */
    constexpr auto INTEREST_RATE_HIKE_PER_INFLATION_POINT = 0.50f;

    /** @brief The central bank cares about jobs too. [cite: 864]
     * @details Weight given to GDP growth gap in interest rate decisions. [cite: 1667] */
    constexpr auto INTEREST_RATE_GROWTH_WEIGHT = 0.30f;
}

/**
 * @namespace Cycle
 * @brief Recession definition, fiscal multipliers, automatic stabilisers. [cite: 836]
 */
namespace Cycle {
    /** @brief Two consecutive quarters. The most feared phrase in economics. [cite: 866, 867]
     * @details Quarterly GDP growth rate that officially defines recession. [cite: 1668] */
    constexpr auto RECESSION_GDP_DECLINE_THRESHOLD = -0.02f;

    /** @brief Every government dollar generates one dollar forty in the economy. [cite: 867]
     * @details GDP units generated per currency unit of government spending. [cite: 1668] */
    constexpr auto MULTIPLIER_FISCAL_STIMULUS = 1.40f;

    /** @brief Borrow too much and your own spending loses its power. [cite: 868]
     * @details Debt-to-GDP ratio above which fiscal multiplier is halved. [cite: 1668, 1669] */
    constexpr auto CROWDING_OUT_DEBT_THRESHOLD = 0.60f;

    /** @brief The shock absorbers built into every modern economy. [cite: 869]
     * @details Fraction of GDP decline offset by automatic stabilisers per quarter. [cite: 1669] */
    constexpr auto AUTOMATIC_STABILISER_STRENGTH = 0.25f;
}

/**
 * @namespace Labour
 * @brief NAIRU, skill premium, strike probability, minimum wage. [cite: 836]
 */
namespace Labour {
    /** @brief Five percent unemployment. The economy's idle engine. [cite: 871]
     * @details Fraction of labour force (NAIRU baseline). [cite: 1669] */
    constexpr auto NATURAL_UNEMPLOYMENT_RATE = 0.05f;

    /** @brief Education pays. Not immediately. Not equally. [cite: 871]
     * @details Wage premium per skill level point above baseline. [cite: 1670] */
    constexpr auto SKILL_PREMIUM_COEFFICIENT = 0.08f;

    /** @brief Small every month. Certain over years. [cite: 872]
     * @details Probability per tick per unionised firm if demands unmet. [cite: 1670] */
    constexpr auto STRIKE_PROBABILITY_BASE = 0.002f;

    /** @brief The most debated number in labour economics. [cite: 873]
     * @details % employment change per % minimum wage increase. [cite: 1670, 1671] */
    constexpr auto MINIMUM_WAGE_EMPLOYMENT_ELASTICITY = -0.15f;
}

/**
 * @namespace Trade
 * @brief Tariff retaliation, import dependency, comparative advantage. [cite: 836]
 */
namespace Trade {
    /** @brief Trade wars are easy to start. Hard to win. [cite: 875]
     * @details Probability that trading partner retaliates per tariff imposed. [cite: 1671] */
    constexpr auto TARIFF_RETALIATION_PROBABILITY = 0.65f;

    /** @brief When half your supply comes from abroad, you are not sovereign. [cite: 876, 877]
     * @details Fraction of consumption from imports that triggers strategic alert. [cite: 1671] */
    constexpr auto IMPORT_DEPENDENCY_VULNERABILITY_THRESHOLD = 0.50f;

    /** @brief Ricardo was right. Specialisation pays. [cite: 877]
     * @details Efficiency bonus for industries where nation has comparative advantage. [cite: 1671, 1672] */
    constexpr auto COMPARATIVE_ADVANTAGE_EXPORT_BONUS = 0.20f;
}

/**
 * @namespace Growth
 * @brief Infrastructure multiplier, R&D thresholds, capital depreciation. [cite: 836]
 */
namespace Growth {
    /** @brief Roads built today pay dividends for decades. [cite: 878]
     * @details GDP growth bonus per 10% improvement in infrastructure quality score. [cite: 1672] */
    constexpr auto INFRASTRUCTURE_PRODUCTIVITY_MULTIPLIER = 0.04f;

    /** @brief Innovation is expensive. Not innovating is more expensive. [cite: 878]
     * @details Cumulative R&D investment units to advance one technology level. [cite: 1672] */
    constexpr auto RD_TECHNOLOGY_UNLOCK_THRESHOLD = 500.0f;

    /** @brief Everything falls apart. The question is how fast. [cite: 879]
     * @details Fraction of infrastructure quality that degrades per year without maintenance. [cite: 1672, 1673] */
    constexpr auto CAPITAL_DEPRECIATION_RATE = 0.02f;
}

/**
 * @namespace Welfare
 * @brief Poverty thresholds, benefit rates, Gini instability coefficient. [cite: 836]
 */
namespace Welfare {
    /** @brief Poverty is relative. So is politics. [cite: 881]
     * @details Fraction of median income below which citizen is classified poor. [cite: 1673] */
    constexpr auto POVERTY_INCOME_THRESHOLD = 0.50f;

    /** @brief Half your wage when you lose your job. Barely enough. By design. [cite: 882, 883]
     * @details Fraction of previous wage replaced by unemployment benefit. [cite: 1673, 1674] */
    constexpr auto UNEMPLOYMENT_BENEFIT_REPLACEMENT_RATE = 0.50f;

    /** @brief Inequality is the slow fuse on every political explosion. [cite: 883]
     * @details Political instability index points added per 0.10 Gini coefficient above 0.30. [cite: 1674] */
    constexpr auto GINI_INSTABILITY_COEFFICIENT = 0.80f;
}

/**
 * @namespace Debt
 * @brief Sovereign borrowing cost, credit spreads, debt sustainability. [cite: 836]
 */
namespace Debt {
    /** @brief Four percent to borrow the world's trust. [cite: 887]
     * @details Annual interest rate on government bonds at AAA credit rating. [cite: 1674] */
    constexpr auto SOVEREIGN_DEBT_INTEREST_BASE = 0.04f;

    /** @brief Each downgrade costs you. Compounding. Forever. [cite: 887]
     * @details Additional annual interest rate per credit rating notch below AAA. [cite: 1674, 1675] */
    constexpr auto CREDIT_SPREAD_PER_NOTCH = 0.005f;
}

/**
 * @namespace Shadow
 * @brief Black market premiums, tax evasion rate, informal economy. [cite: 836]
 */
namespace Shadow {
    /** @brief Where the government sets prices, the market sets different ones. [cite: 889]
     * @details Multiplier on controlled price to get black market price. [cite: 1675] */
    constexpr auto BLACK_MARKET_PRICE_PREMIUM = 1.50f;

    /** @brief A quarter of the underground economy pays nothing. [cite: 890]
     * @details Fraction of shadow economy transactions that evade all taxation. [cite: 1675, 1676] */
    constexpr auto SHADOW_ECONOMY_TAX_EVASION_RATE = 0.25f;
}

/**
 * @namespace Env
 * @brief Pollution health costs, renewable learning curve. [cite: 836]
 */
namespace Env {
    /** @brief Pollution is never free. It just bills you later. [cite: 892]
     * @details Currency units of healthcare cost per pollution unit per tile per year. [cite: 1676] */
    constexpr auto POLLUTION_HEALTH_COST_MULTIPLIER = 2.50f;

    /** @brief The learning curve that changes the world. [cite: 893]
     * @details Annual fractional cost reduction for solar and wind per technology level. [cite: 1676, 1677] */
    constexpr auto RENEWABLE_COST_REDUCTION_RATE = 0.07f;
}

/**
 * @namespace Sim
 * @brief Tick calibration, citizen memory, approval weights, growth caps. [cite: 836]
 */
namespace Sim {
    /** @brief The heartbeat of the simulation. [cite: 895]
     * @details Simulation ticks (at 60 ticks/second real time = 12 real seconds per month). [cite: 1677] */
    constexpr auto TICKS_PER_GAME_MONTH = 720;

    /** @brief Citizens judge you on what you did lately, not what you did once. [cite: 896]
     * @details Months of economic experience weighted in citizen approval calculation. [cite: 1677] */
    constexpr auto CITIZEN_ECONOMIC_MEMORY_MONTHS = 12;

    /** @brief Ten percent growth is a miracle. Or a mirage. [cite: 897]
     * @details Maximum annual GDP growth rate before bubble/overheating flag fires. [cite: 1677, 1678] */
    constexpr auto MAX_PLAUSIBLE_GDP_GROWTH_RATE = 0.10f;

    /** @brief It's the economy. Almost half of it, anyway. [cite: 898]
     * @details Fraction of total approval determined by economic factors. [cite: 1678] */
    constexpr auto APPROVAL_ECONOMIC_WEIGHT = 0.45f;
}