#pragma once
#include "main.h"

#include "gcem.hpp"
#include "helpers/freertos.hpp"
#include "leds.hpp"

#include "util/Button.hpp"
#include "wrappers/Task.hpp"

/// encoder button is handled here
class ButtonHandler : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    static constexpr auto ButtonSamplingInterval = 10.0_ms;
    static constexpr auto LongPressTime = 500.0_ms;

    ButtonHandler(LedFading &ledFading)
        : TaskWithMemberFunctionBase("buttonPollingTask", 128, osPriorityBelowNormal6), //
          ledFading(ledFading)
    {
        constexpr auto Divisor =
            (util::Button::DebounceTime / ButtonSamplingInterval).getMagnitude();
        static_assert(gcem::ceil(Divisor) == Divisor,
                      "The update time is not an even divisor of debounce time!");
    }

protected:
    [[noreturn]] void taskMain() override;

private:
    LedFading &ledFading;

    util::Button encoderButton{{EncoderButton_GPIO_Port, EncoderButton_Pin},
                               LongPressTime,
                               false,
                               [this](auto action) { encoderButtonCallback(action); }};

    void encoderButtonCallback(util::Button::Action action);
};