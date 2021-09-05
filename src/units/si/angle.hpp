#pragma once

#include "units/si.hpp"

namespace units::si
{
using Angle = Value<SiUnit<0, 0, 0, 0, 0, 0, 0>>;
} // namespace units::si

constexpr units::si::Angle operator"" _rad(const long double magnitude)
{
    return units::si::Angle(magnitude);
}
