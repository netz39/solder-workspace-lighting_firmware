#pragma once

#include "adc.h"
#include "spi.h"
#include "tim.h"

#include "analog_to_digital/AnalogToDigital.hpp"
#include "encoder/Encoder.hpp"
#include "encoder/EncoderButtonHandler.hpp"
#include "led_control/LedFading.hpp"
#include "oled_display/OledDisplay.hpp"

class Application
{
public:
    static constexpr auto AdcPeripherie = &hadc1;
    static constexpr auto EncoderTimer = &htim1;
    static constexpr auto LedTimer = &htim2;
    static constexpr auto DisplaySpiPeripherie = &hspi1;

    Application();
    [[noreturn]] void run();

    static Application &getApplicationInstance();
    static void timeoutCallback(TimerHandle_t timer);

private:
    static inline Application *instance{nullptr};

    LedFading ledFading{LedTimer, &timeoutCallback};

    AnalogToDigital::LedTemperatures ledTemperatures{0.0_degC, 0.0_degC, 0.0_degC, 0.0_degC};
    bool isOverTemperature = false;
    AnalogToDigital analogToDigital{AdcPeripherie, ledTemperatures, isOverTemperature, ledFading};

    EncoderButtonHandler encoderButtonHandler{ledFading};
    Encoder encoder{EncoderTimer, isOverTemperature, ledFading};
    OledDisplay oledDisplay{DisplaySpiPeripherie, ledTemperatures, ledFading};
};