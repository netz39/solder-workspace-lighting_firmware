#pragma once

#include "units/si/temperature.hpp"
#include <array>

using units::si::Temperature;

constexpr auto TemperatureChannelCount = 4;
extern std::array<Temperature, TemperatureChannelCount> ledTemperatures;