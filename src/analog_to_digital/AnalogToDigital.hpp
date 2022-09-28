#pragma once

#include "units/si/frequency.hpp"
#include "units/si/resistance.hpp"
#include "units/si/scalar.hpp"
#include "units/si/temperature.hpp"
#include "units/si/voltage.hpp"

#include "core/SafeAssert.h"
#include "led_control/LedFading.hpp"
#include "wrappers/Task.hpp"

#include <array>

using units::si::Temperature;

class AnalogToDigital : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    static constexpr auto TemperatureChannelCount = 4;

    static constexpr auto TotalChannelNumber = TemperatureChannelCount + 1; // vref
    static constexpr auto VrefChannel = TotalChannelNumber - 1;

    static constexpr auto AdcTaskFrequency = 100.0_Hz;
    static constexpr auto AdcResolution = 12;
    static constexpr auto MaxAdcValue = (1 << AdcResolution) - 1;

    static constexpr auto NtcBetaValue = 3380.0f;
    static constexpr auto NtcNominalTemperature = 25.0_degC;
    static constexpr auto NtcResistanceAtNominalTemperature = 10_kOhm;
    static constexpr auto NtcSecondResistor = 10_kOhm;

    static constexpr auto MaximumTemperatureLowerThreshold = 70.0_degC;
    static constexpr auto MaximumTemperatureUpperThreshold = 80.0_degC;

    using LedTemperatures = std::array<Temperature, TemperatureChannelCount>;

    AnalogToDigital(ADC_HandleTypeDef *peripherie, LedTemperatures &ledTemperatures,
                    bool &isOverTemperature, LedFading &ledFading)
        : TaskWithMemberFunctionBase("adcTask", 512, osPriorityBelowNormal4), //
          peripherie(peripherie),                                             //
          ledTemperatures(ledTemperatures),                                   //
          isOverTemperature(isOverTemperature),                               //
          ledFading(ledFading)
    {
        SafeAssert(peripherie != nullptr);
    }

    void conversionCompleteCallback();

protected:
    [[noreturn]] void taskMain() override;

private:
    ADC_HandleTypeDef *peripherie = nullptr;
    LedTemperatures &ledTemperatures;
    bool &isOverTemperature;
    LedFading &ledFading;

    units::si::Voltage referenceVoltage = 3.3_V;
    std::array<uint16_t, TotalChannelNumber> adcResults{};

    void calculateReferenceVoltage();
    void calibrateAdc();
    void startConversion();
    void waitUntilConversionFinished();
    void calculateTemperatures();
    void checkOverTemperature();

    template <class T>
    constexpr auto physicalQuantityFromAdcResult(const uint16_t adcResult, const T multiplier)
    {
        return (adcResult * referenceVoltage * multiplier) / MaxAdcValue;
    }

    template <class T>
    void updateFastLowpass(T &oldValue, const T newSample, const uint8_t sampleCount)
    {
        oldValue += (newSample - oldValue) / static_cast<float>(sampleCount);
    }
};