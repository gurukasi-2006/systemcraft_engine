#pragma once

/**
 * @file economic_constants.hpp
 * @brief Defines the compile-time economic constants, elasticities, and dampening factors.
 * @details Contains 48 constants divided across 12 domain namespaces. Override via GlobalConfig at runtime for scenarios.
 */

/**
 * @namespace Tax
 * @brief All taxation: income, corporate, VAT, property, royalties, excise.
 */
namespace Tax {
    /** @brief The contract between citizen and state.
     * @details Fraction of gross income.*/
    constexpr auto BASE_INCOME_TAX_RATE = 0.20f;

    /** @brief The price of doing business in your nation.
     * @details Fraction of industry net profit. */
    constexpr auto BASE_CORPORATE_TAX_RATE = 0.25f;

    /** @brief The invisible tax. Regressive by design.
     * @details Fraction added to consumer goods price.*/
    constexpr auto BASE_VAT_RATE = 0.15f;

    /** @brief The tax that shapes cities.
     * @details Fraction of building market valuation per year.*/
    constexpr auto BASE_PROPERTY_TAX_RATE = 0.01f;

    /** @brief The sovereign's share of the ground beneath.
     * @details Fraction of gross extraction value. */
    constexpr auto RESOURCE_EXTRACTION_ROYALTY = 0.08f;

    /** @brief Zero by default. The cost of ignoring it is measured in terrain.
     * @details Currency units per tonne CO2-equivalent per tick. */
    constexpr auto CARBON_TAX_PER_TONNE = 0.0f;

    /** @brief The government's most reliable sin revenue.
     * @details Fraction of retail price as excise duty.*/
    constexpr auto TOBACCO_EXCISE_RATE = 0.55f;

    /** @brief Enough to collect. Not enough to ban.
     * @details Fraction of retail price as excise duty.*/
    constexpr auto ALCOHOL_EXCISE_RATE = 0.35f;
}

/**
 * @namespace Market
 * @brief Price elasticities, bubble detection, subsidy pass-through.
 */
namespace Market {
    /** @brief People will pay almost anything not to starve.
     * @details % demand change per % price change.*/
    constexpr auto PRICE_ELASTICITY_FOOD = -0.30f;

    /** @brief Expensive energy hurts everything, slowly.
     * @details % demand change per % price change.*/
    constexpr auto PRICE_ELASTICITY_ENERGY = -0.45f;

    /** @brief Luxuries are optional. Citizens know this.
     * @details % demand change per % price change.*/
    constexpr auto PRICE_ELASTICITY_CONSUMER_GOODS = -1.20f;

    /** @brief Shelter is not optional. Neither is political backlash.
     * @details % demand change per % rent increase. */
    constexpr auto PRICE_ELASTICITY_HOUSING_RENT = -0.20f;

    /** @brief Eight percent a month. The market is lying to you.
     * @details Monthly price growth rate fraction above which bubble warning fires. */
    constexpr auto BUBBLE_DETECTION_GROWTH_THRESHOLD = 0.08f;

    /** @brief Markets fear uncertainty more than bad news.
     * @details Weight of price volatility in confidence index (0-1). */
    constexpr auto MARKET_CONFIDENCE_VOLATILITY_WEIGHT = 0.60f;

    /** @brief Half of every subsidy disappears before it helps anyone.
     * @details Fraction of subsidy that reaches consumer as lower price.*/
    constexpr auto SUBSIDY_CONSUMER_PASSTHROUGH_RATE = 0.55f;
}

/**
 * @namespace Monetary
 * @brief Inflation targeting, interest rates, wage-price spiral, dampening.
 */
namespace Monetary {
    /** @brief Two percent. The most consequential number in economics.
     * @details Annual fraction (monthly: 0.00167f).*/
    constexpr auto TARGET_INFLATION_RATE = 0.02f;

    /** @brief The central bank's patience, measured precisely.
     * @details Fraction either side of target before central bank acts.*/
    constexpr auto INFLATION_TOLERANCE_BAND = 0.01f;

    /** @brief The economy self-corrects. Slowly. Painfully.
     * @details Fraction by which excess inflation reduces itself per month. */
    constexpr auto INFLATION_DAMPENING_FACTOR = 0.15f;

    /** @brief Workers ask for more. Firms charge more. Repeat.
     * @details Fraction of CPI inflation that feeds into wage demands each month. */
    constexpr auto WAGE_PRICE_SPIRAL_COEFFICIENT = 0.40f;

    /** @brief The Taylor Rule, simplified for a nation-builder.
     * @details Basis points interest rate rise per 1% inflation above target. */
    constexpr auto INTEREST_RATE_HIKE_PER_INFLATION_POINT = 0.50f;

    /** @brief The central bank cares about jobs too. 
     * @details Weight given to GDP growth gap in interest rate decisions. */
    constexpr auto INTEREST_RATE_GROWTH_WEIGHT = 0.30f;
}

/**
 * @namespace Cycle
 * @brief Recession definition, fiscal multipliers, automatic stabilisers.
 */
namespace Cycle {
    /** @brief Two consecutive quarters. The most feared phrase in economics.
     * @details Quarterly GDP growth rate that officially defines recession. */
    constexpr auto RECESSION_GDP_DECLINE_THRESHOLD = -0.02f;

    /** @brief Every government dollar generates one dollar forty in the economy.
     * @details GDP units generated per currency unit of government spending. */
    constexpr auto MULTIPLIER_FISCAL_STIMULUS = 1.40f;

    /** @brief Borrow too much and your own spending loses its power. 
     * @details Debt-to-GDP ratio above which fiscal multiplier is halved. */
    constexpr auto CROWDING_OUT_DEBT_THRESHOLD = 0.60f;

    /** @brief The shock absorbers built into every modern economy.
     * @details Fraction of GDP decline offset by automatic stabilisers per quarter. */
    constexpr auto AUTOMATIC_STABILISER_STRENGTH = 0.25f;
}

/**
 * @namespace Labour
 * @brief NAIRU, skill premium, strike probability, minimum wage.
 */
namespace Labour {
    /** @brief Five percent unemployment. The economy's idle engine.
     * @details Fraction of labour force (NAIRU baseline). */
    constexpr auto NATURAL_UNEMPLOYMENT_RATE = 0.05f;

    /** @brief Education pays. Not immediately. Not equally.
     * @details Wage premium per skill level point above baseline. */
    constexpr auto SKILL_PREMIUM_COEFFICIENT = 0.08f;

    /** @brief Small every month. Certain over years.
     * @details Probability per tick per unionised firm if demands unmet. */
    constexpr auto STRIKE_PROBABILITY_BASE = 0.002f;

    /** @brief The most debated number in labour economics.
     * @details % employment change per % minimum wage increase. */
    constexpr auto MINIMUM_WAGE_EMPLOYMENT_ELASTICITY = -0.15f;
}

/**
 * @namespace Trade
 * @brief Tariff retaliation, import dependency, comparative advantage.
 */
namespace Trade {
    /** @brief Trade wars are easy to start. Hard to win.
     * @details Probability that trading partner retaliates per tariff imposed. */
    constexpr auto TARIFF_RETALIATION_PROBABILITY = 0.65f;

    /** @brief When half your supply comes from abroad, you are not sovereign.
     * @details Fraction of consumption from imports that triggers strategic alert. */
    constexpr auto IMPORT_DEPENDENCY_VULNERABILITY_THRESHOLD = 0.50f;

    /** @brief Ricardo was right. Specialisation pays.
     * @details Efficiency bonus for industries where nation has comparative advantage. */
    constexpr auto COMPARATIVE_ADVANTAGE_EXPORT_BONUS = 0.20f;
}

/**
 * @namespace Growth
 * @brief Infrastructure multiplier, R&D thresholds, capital depreciation.
 */
namespace Growth {
    /** @brief Roads built today pay dividends for decades.
     * @details GDP growth bonus per 10% improvement in infrastructure quality score. */
    constexpr auto INFRASTRUCTURE_PRODUCTIVITY_MULTIPLIER = 0.04f;

    /** @brief Innovation is expensive. Not innovating is more expensive.
     * @details Cumulative R&D investment units to advance one technology level. */
    constexpr auto RD_TECHNOLOGY_UNLOCK_THRESHOLD = 500.0f;

    /** @brief Everything falls apart. The question is how fast.
     * @details Fraction of infrastructure quality that degrades per year without maintenance.*/
    constexpr auto CAPITAL_DEPRECIATION_RATE = 0.02f;
}

/**
 * @namespace Welfare
 * @brief Poverty thresholds, benefit rates, Gini instability coefficient.
 */
namespace Welfare {
    /** @brief Poverty is relative. So is politics.
     * @details Fraction of median income below which citizen is classified poor. */
    constexpr auto POVERTY_INCOME_THRESHOLD = 0.50f;

    /** @brief Half your wage when you lose your job. Barely enough. By design.
     * @details Fraction of previous wage replaced by unemployment benefit. */
    constexpr auto UNEMPLOYMENT_BENEFIT_REPLACEMENT_RATE = 0.50f;

    /** @brief Inequality is the slow fuse on every political explosion.
     * @details Political instability index points added per 0.10 Gini coefficient above 0.30. */
    constexpr auto GINI_INSTABILITY_COEFFICIENT = 0.80f;
}

/**
 * @namespace Debt
 * @brief Sovereign borrowing cost, credit spreads, debt sustainability.
 */
namespace Debt {
    /** @brief Four percent to borrow the world's trust.
     * @details Annual interest rate on government bonds at AAA credit rating. */
    constexpr auto SOVEREIGN_DEBT_INTEREST_BASE = 0.04f;

    /** @brief Each downgrade costs you. Compounding. Forever. 
     * @details Additional annual interest rate per credit rating notch below AAA. */
    constexpr auto CREDIT_SPREAD_PER_NOTCH = 0.005f;
}

/**
 * @namespace Shadow
 * @brief Black market premiums, tax evasion rate, informal economy. 
 */
namespace Shadow {
    /** @brief Where the government sets prices, the market sets different ones. 
     * @details Multiplier on controlled price to get black market price. */
    constexpr auto BLACK_MARKET_PRICE_PREMIUM = 1.50f;

    /** @brief A quarter of the underground economy pays nothing. 
     * @details Fraction of shadow economy transactions that evade all taxation. */
    constexpr auto SHADOW_ECONOMY_TAX_EVASION_RATE = 0.25f;
}

/**
 * @namespace Env
 * @brief Pollution health costs, renewable learning curve.
 */
namespace Env {
    /** @brief Pollution is never free. It just bills you later.
     * @details Currency units of healthcare cost per pollution unit per tile per year. */
    constexpr auto POLLUTION_HEALTH_COST_MULTIPLIER = 2.50f;

    /** @brief The learning curve that changes the world.
     * @details Annual fractional cost reduction for solar and wind per technology level. */
    constexpr auto RENEWABLE_COST_REDUCTION_RATE = 0.07f;
}

/**
 * @namespace Sim
 * @brief Tick calibration, citizen memory, approval weights, growth caps. 
 */
namespace Sim {
    /** @brief The heartbeat of the simulation.
     * @details Simulation ticks (at 60 ticks/second real time = 12 real seconds per month). */
    constexpr auto TICKS_PER_GAME_MONTH = 720;

    /** @brief Citizens judge you on what you did lately, not what you did once.
     * @details Months of economic experience weighted in citizen approval calculation. */
    constexpr auto CITIZEN_ECONOMIC_MEMORY_MONTHS = 12;

    /** @brief Ten percent growth is a miracle. Or a mirage.
     * @details Maximum annual GDP growth rate before bubble/overheating flag fires. */
    constexpr auto MAX_PLAUSIBLE_GDP_GROWTH_RATE = 0.10f;

    /** @brief It's the economy. Almost half of it, anyway.
     * @details Fraction of total approval determined by economic factors. */
    constexpr auto APPROVAL_ECONOMIC_WEIGHT = 0.45f;
}