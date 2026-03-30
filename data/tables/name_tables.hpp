// In data/tables/name_tables.hpp
#pragma once
#include <array>
#include <string_view>

namespace NameTables {

constexpr std::array<std::string_view, 64> MALE_FIRST = {
    "Arlen","Bren","Caius","Dael","Emyn","Farel","Goran","Haren",
    "Idris","Jorin","Kavan","Laren","Mavel","Navar","Ovel","Paren",
    "Dovan","Edrik","Emric","Faruk","Gavril","Havar","Ilvar","Joven",
    "Kael","Kelan","Koryn","Lerik","Lodan","Lyren","Makan","Merik",
    "Kasim","Levan","Loris","Maren","Mikos","Nael","Nestor","Odan",
    "Orek","Oswin","Pavik","Peran","Rael","Ravan","Revik","Rodan",
    "Rylen","Soren","Stellan","Tavar","Tevan","Torin","Ulvan","Vadek",
    "Varek","Velen","Vespar","Vorin","Waren","Xavan","Yoran","Zoren"
};

constexpr std::array<std::string_view, 64> FEMALE_FIRST = {
    "Aela","Bera","Calla","Dael","Elva","Faren","Gala","Halen",
    "Isel","Jora","Kala","Lavel","Maren","Navel","Ovel","Parel",
    "Davan","Edlyn","Elara","Favel","Gavra","Havel","Ilara","Ivara",
    "Jael","Kavel","Kelis","Koryn","Lanis","Levan","Lyra","Mavel",
    "Karis","Laren","Lenis","Lira","Mira","Moven","Nara","Neven",
    "Oela","Olen","Oryn","Parel","Rael","Ravel","Relan","Risa",
    "Riven","Sael","Sevan","Solan","Tael","Terin","Thela","Torin",
    "Ulara","Vael","Velen","Vira","Wela","Xara","Yara","Zela"
};

constexpr std::array<std::string_view, 64> SURNAMES = {
    "Aldren","Arken","Arven","Balden","Barken","Beldan","Boldan","Borven",
    "Braven","Calden","Carven","Celdan","Colden","Corven","Dalden","Darken",
    "Dovan","Drevak","Edrak","Eldrak","Elven","Farken","Feldan","Galden",
    "Garven","Geldan","Gorven","Halden","Harken","Heldan","Holden","Jorven",
    "Karven","Kelden","Korven","Landen","Larken","Loden","Malden","Marken",
    "Morven","Nalden","Narken","Norven","Palden","Parken","Peldan","Rolden",
    "Roven","Ryken","Seldan","Sorven","Starven","Talden","Tarken","Teldan",
    "Torven","Ulden","Valden","Varken","Veldan","Volden","Walden","Zorven"
};

}// namespace name is NameTables