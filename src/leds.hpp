#pragma once

#include "gcem/include/gcem.hpp"

#include <array>

constexpr auto NumberOfLeds = 4;
constexpr auto PwmResolutionInBit = 16;

constexpr auto GammaFactor = 2.5;
constexpr auto MaximumIn = 100;
constexpr auto MaximumOut = (1 << PwmResolutionInBit) - 1;

using GammaTable = std::array<uint16_t, MaximumIn + 1>;

constexpr GammaTable createGammaTable()
{
    GammaTable gammaTable{};

    for (auto i = 0; i <= MaximumIn; i++)
    {
        const auto Logarithm =
            gcem::pow(static_cast<float>(i) / static_cast<float>(MaximumIn), GammaFactor);

        gammaTable[i] = gcem::round(Logarithm * MaximumOut);
    }

    return gammaTable;
}

constexpr GammaTable GammaLUT = createGammaTable();

constexpr auto Val50 = GammaLUT[50];
constexpr auto Val75 = GammaLUT[75];
constexpr auto Val95 = GammaLUT[95];
constexpr auto Val96 = GammaLUT[96];
constexpr auto Val97 = GammaLUT[97];
constexpr auto Val98 = GammaLUT[98];
constexpr auto Val99 = GammaLUT[99];
constexpr auto Val100 = GammaLUT[100];