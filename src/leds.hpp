#pragma once
#include <cstdint>

extern uint8_t targetLedPercentage;

enum class FadingState
{
    Normal,
    Standby
};

constexpr auto NumberOfLeds = 4;
constexpr auto MinPercentage = 0;
constexpr auto MaxPercentage = 100;
constexpr auto DefaultPercentage = 80;