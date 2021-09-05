#pragma once

#include "units/offset.hpp"
#include "units/scale.hpp"
#include "units/si_unit.hpp"
#include <cstdint>
#include <type_traits>

namespace units
{
template <typename SiUnit>
class Value
{
private:
    float magnitude{0};

public:
    using Unit = SiUnit;

    constexpr explicit Value(const float magnitude) noexcept : magnitude{magnitude}
    {
    }

    template <typename Type = float>
    constexpr Type getMagnitude() const noexcept
    {
        if constexpr (std::is_same<Type, float>::value)
            return magnitude;
        else
            return static_cast<Type>(magnitude);
    }

    template <typename Type = float, typename ScaleSiUnit>
    constexpr Type getMagnitude(const Scale<ScaleSiUnit> scale) const noexcept
    {
        static_assert(std::is_same<SiUnit, ScaleSiUnit>::value,
                      "Scale has incompatible underlying SI unit");

        if constexpr (std::is_same<Type, float>::value)
            return magnitude * scale.getScalingFactor();
        else
            return static_cast<Type>(magnitude * scale.getScalingFactor());
    }

    template <typename Type = float, typename OffsetSiUnit>
    constexpr Type getMagnitude(const Offset<OffsetSiUnit> offset) const noexcept
    {
        static_assert(std::is_same<SiUnit, OffsetSiUnit>::value,
                      "Offset has incompatible underlying SI unit");

        if constexpr (std::is_same<Type, float>::value)
            return magnitude + offset.getOffset();
        else
            return static_cast<Type>(magnitude + offset.getOffset());
    }

    template <typename Type = float>
    constexpr void setMagnitude(Type magnitude) noexcept
    {
        if constexpr (std::is_same<Type, float>::value)
            this->magnitude = magnitude;
        else
            this->magnitude = static_cast<Type>(magnitude);
    }
};

// Scalar operations:

template <int M, int Kg, int S, int A, int K, int Mo, int C, typename T>
constexpr Value<SiUnit<M, Kg, S, A, K, Mo, C>>
operator*(const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &lhs, const T &rhs)
{
    return Value<SiUnit<M, Kg, S, A, K, Mo, C>>(lhs.getMagnitude() * rhs);
}

template <int M, int Kg, int S, int A, int K, int Mo, int C, typename T>
constexpr Value<SiUnit<M, Kg, S, A, K, Mo, C>>
operator*(const T &lhs, const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &rhs)
{
    return Value<SiUnit<M, Kg, S, A, K, Mo, C>>(rhs.getMagnitude() * lhs);
}

template <int M, int Kg, int S, int A, int K, int Mo, int C, typename T>
constexpr Value<SiUnit<M, Kg, S, A, K, Mo, C>>
operator/(const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &lhs, const T &rhs)
{
    return Value<SiUnit<M, Kg, S, A, K, Mo, C>>(lhs.getMagnitude() / rhs);
}

// Arithmetic operators:

template <int M, int Kg, int S, int A, int K, int Mo, int C>
constexpr Value<SiUnit<M, Kg, S, A, K, Mo, C>>
operator-(const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &op) noexcept
{
    return Value<SiUnit<M, Kg, S, A, K, Mo, C>>(-op.getMagnitude());
}

template <int M, int Kg, int S, int A, int K, int Mo, int C>
constexpr Value<SiUnit<M, Kg, S, A, K, Mo, C>>
operator+(const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &lhs,
          const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &rhs) noexcept
{
    return Value<SiUnit<M, Kg, S, A, K, Mo, C>>(lhs.getMagnitude() + rhs.getMagnitude());
}

template <int M, int Kg, int S, int A, int K, int Mo, int C>
constexpr Value<SiUnit<M, Kg, S, A, K, Mo, C>>
operator-(const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &lhs,
          const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &rhs) noexcept
{
    return Value<SiUnit<M, Kg, S, A, K, Mo, C>>(lhs.getMagnitude() - rhs.getMagnitude());
}

template <int M1, int Kg1, int S1, int A1, int K1, int Mo1, int C1, int M2, int Kg2, int S2, int A2,
          int K2, int Mo2, int C2>
constexpr Value<SiUnit<M1 + M2, Kg1 + Kg2, S1 + S2, A1 + A2, K1 + K2, Mo1 + Mo2, C1 + C2>>
operator*(const Value<SiUnit<M1, Kg1, S1, A1, K1, Mo1, C1>> &lhs,
          const Value<SiUnit<M2, Kg2, S2, A2, K2, Mo2, C2>> &rhs) noexcept
{
    return Value<SiUnit<M1 + M2, Kg1 + Kg2, S1 + S2, A1 + A2, K1 + K2, Mo1 + Mo2, C1 + C2>>(
        lhs.getMagnitude() * rhs.getMagnitude());
}

template <int M1, int Kg1, int S1, int A1, int K1, int Mo1, int C1, int M2, int Kg2, int S2, int A2,
          int K2, int Mo2, int C2>
constexpr Value<SiUnit<M1 - M2, Kg1 - Kg2, S1 - S2, A1 - A2, K1 - K2, Mo1 - Mo2, C1 - C2>>
operator/(const Value<SiUnit<M1, Kg1, S1, A1, K1, Mo1, C1>> &lhs,
          const Value<SiUnit<M2, Kg2, S2, A2, K2, Mo2, C2>> &rhs) noexcept
{
    return Value<SiUnit<M1 - M2, Kg1 - Kg2, S1 - S2, A1 - A2, K1 - K2, Mo1 - Mo2, C1 - C2>>(
        lhs.getMagnitude() / rhs.getMagnitude());
}

// Combined operators:

template <int M, int Kg, int S, int A, int K, int Mo, int C>
constexpr Value<SiUnit<M, Kg, S, A, K, Mo, C>> &
operator+=(Value<SiUnit<M, Kg, S, A, K, Mo, C>> &lhs,
           const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &rhs) noexcept
{
    lhs.setMagnitude(lhs.getMagnitude() + rhs.getMagnitude());
    return lhs;
}

template <int M, int Kg, int S, int A, int K, int Mo, int C>
constexpr Value<SiUnit<M, Kg, S, A, K, Mo, C>> &
operator-=(Value<SiUnit<M, Kg, S, A, K, Mo, C>> &lhs,
           const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &rhs) noexcept
{
    lhs.setMagnitude(lhs.getMagnitude() - rhs.getMagnitude());
    return lhs;
}

template <int M, int Kg, int S, int A, int K, int Mo, int C>
constexpr Value<SiUnit<M, Kg, S, A, K, Mo, C>>
sign(const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &v) noexcept
{
    return Value<SiUnit<M, Kg, S, A, K, Mo, C>>(v.getMagnitude() > 0.0 ? 1.0 : -1.0);
}

template <int M, int Kg, int S, int A, int K, int Mo, int C>
int signInt(const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &v) noexcept
{
    return v.getMagnitude() > 0.0 ? 1 : -1;
}

// Comparison operators:

template <int M, int Kg, int S, int A, int K, int Mo, int C>
constexpr bool operator<(const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &lhs,
                         const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &rhs)
{
    return lhs.getMagnitude() < rhs.getMagnitude();
}

template <int M, int Kg, int S, int A, int K, int Mo, int C>
constexpr bool operator<=(const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &lhs,
                          const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &rhs)
{
    return lhs.getMagnitude() <= rhs.getMagnitude();
}

template <int M, int Kg, int S, int A, int K, int Mo, int C>
constexpr bool operator>(const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &lhs,
                         const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &rhs)
{
    return lhs.getMagnitude() > rhs.getMagnitude();
}

template <int M, int Kg, int S, int A, int K, int Mo, int C>
constexpr bool operator>=(const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &lhs,
                          const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &rhs)
{
    return lhs.getMagnitude() >= rhs.getMagnitude();
}

template <int M, int Kg, int S, int A, int K, int Mo, int C>
constexpr bool operator==(const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &lhs,
                          const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &rhs)
{
    return lhs.getMagnitude() == rhs.getMagnitude();
}

template <int M, int Kg, int S, int A, int K, int Mo, int C>
constexpr bool operator!=(const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &lhs,
                          const Value<SiUnit<M, Kg, S, A, K, Mo, C>> &rhs)
{
    return lhs.getMagnitude() != rhs.getMagnitude();
}
} // namespace units
