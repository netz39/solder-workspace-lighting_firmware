#pragma once

#include "analog_to_digital/AnalogToDigital.hpp"
#include "core/SafeAssert.h"
#include "helpers/freertos.hpp"
#include "util/gpio.hpp"
#include "wrappers/Task.hpp"

#include "SSD1306_SPI.hpp"
#include "oled-driver/Display.hpp"
#include "oled-driver/Renderer.hpp"

class OledDisplay : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    static constexpr size_t OledWidth = SSD1306_SPI::OledWidth;
    static constexpr size_t OledPages = SSD1306_SPI::OledPages;

    static constexpr auto TaskFrequency = 100.0_Hz;
    static constexpr auto TimeThresholdToShowCountdown = 9.0_min;

    OledDisplay(SPI_HandleTypeDef *peripherie, AnalogToDigital::LedTemperatures &ledTemperatures,
                LedFading &ledFading)
        : TaskWithMemberFunctionBase("oledDisplayTask", 512, osPriorityNormal4), //
          peripherie(peripherie),                                                //
          ledTemperatures(ledTemperatures),                                      //
          ledFading(ledFading)
    {
        SafeAssert(peripherie != nullptr);
    }

    void notifyRenderTaskFromISR();

protected:
    [[noreturn]] void taskMain() override;

private:
    SPI_HandleTypeDef *peripherie = nullptr;
    AnalogToDigital::LedTemperatures &ledTemperatures;
    LedFading &ledFading;

    SSD1306_SPI ssdi{peripherie};
    Display display{ssdi};
    Renderer renderer{OledWidth, OledPages, display};

    util::Gpio displayReset{DisplayReset_GPIO_Port, DisplayReset_Pin};

    void initDisplay();

    void waitForTXComplete();
    void getTemperaturePrint(const units::si::Temperature &temperature, char *buffer,
                             size_t bufferSize);
    void drawDisplay();

    static constexpr auto MaximumChars = 22;
    char buffer[MaximumChars]{};
};
