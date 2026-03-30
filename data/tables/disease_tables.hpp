// data/tables/disease_tables.hpp
#pragma once
#include <array>
#include <string_view>

struct DiseaseEntry {
    std::string_view name;
    float            phys_drain;
    float            mental_drain;
};

constexpr std::array<DiseaseEntry, 40> DISEASE_TABLE = {{
    { "Greylung Fever",      0.045f, 0.010f },  // D01
    { "Ashwhooping",         0.018f, 0.005f },  // D02
    { "Ironbreath",          0.090f, 0.020f },  // D03
    { "Pale Sickness",       0.052f, 0.008f },  // D04
    { "Dustcough",           0.022f, 0.003f },  // D05
    { "Vaultfever",          0.130f, 0.040f },  // D06
    { "Gutter Flux",         0.048f, 0.012f },  // D07
    { "Brackwater Rot",      0.085f, 0.018f },  // D08
    { "Stomachfire",         0.025f, 0.005f },  // D09
    { "Hollow Gut",          0.055f, 0.030f },  // D10
    { "Blackwater Fever",    0.120f, 0.025f },  // D11
    { "Marsh Plague",        0.095f, 0.035f },  // D12
    { "Redmite Fever",       0.050f, 0.015f },  // D13
    { "Sandfly Wasting",     0.080f, 0.020f },  // D14
    { "Crawler Pox",         0.042f, 0.010f },  // D15
    { "Duskfly Paralysis",   0.140f, 0.050f },  // D16
    { "Hearthwaste",         0.038f, 0.005f },  // D17
    { "Greymind Fatigue",    0.012f, 0.055f },  // D18
    { "Bloodstilling",       0.075f, 0.010f },  // D19
    { "Softbone Disease",    0.020f, 0.002f },  // D20
    { "Veinfire",            0.045f, 0.008f },  // D21
    { "Coreworking Strain",  0.015f, 0.018f },  // D22
    { "Deepgrief Disorder",  0.010f, 0.060f },  // D23
    { "Hollow State",        0.015f, 0.090f },  // D24
    { "Fearlock Syndrome",   0.012f, 0.065f },  // D25
    { "Warcall Trauma",      0.018f, 0.100f },  // D26
    { "Crushwork Syndrome",  0.010f, 0.040f },  // D27
    { "Ironlack Anaemia",    0.022f, 0.020f },  // D28
    { "Nightblind Wasting",  0.038f, 0.008f },  // D29
    { "Ricksbone",           0.035f, 0.005f },  // D30
    { "Saltfever",           0.018f, 0.012f },  // D31
    { "Greensick",           0.015f, 0.015f },  // D32
    { "Slaglung",            0.082f, 0.010f },  // D33
    { "Coldwater Palsy",     0.044f, 0.022f },  // D34
    { "Chemburn Rash",       0.050f, 0.015f },  // D35
    { "Leadhead Toxin",      0.078f, 0.065f },  // D36
    { "Ashfall Sickness",    0.088f, 0.022f },  // D37
    { "Black Harvest",       0.135f, 0.035f },  // D38
    { "Ironrust Plague",     0.145f, 0.045f },  // D39
    { "The Greying",         0.150f, 0.080f },  // D40
}};