#pragma once

#include "OledDisplay.hpp"
#include "core/SafeAssert.h"
#include "main.h"
#include "oled-driver/SSD1306Interface.hpp"
#include "spi.h"

//--------------------------------------------------------------------------------------------------
//! SPI interface to a SSD1305/6 display controller.
class SSD1306_SPI : public SSD1306Interface
{
public:
    static constexpr size_t OledWidth = 128;
    static constexpr size_t OledPages = 4;

    explicit SSD1306_SPI(SPI_HandleTypeDef *peripherie) : SSD1306Interface(), peripherie(peripherie)
    {
        SafeAssert(peripherie != nullptr);
    }

    void writeCommand(uint8_t cmd) override
    {
        setCommandPin();
        setChipSelect(true);

        HAL_SPI_Transmit_DMA(peripherie, &cmd, 1);
        waitForTXComplete();

        setChipSelect(false);
    }

    //--------------------------------------------------------------------------------------------------
    void writeData(uint8_t data) override
    {
        setDataPin();
        setChipSelect(true);

        HAL_SPI_Transmit_DMA(peripherie, &data, 1);
        waitForTXComplete();

        setChipSelect(false);
    }

    //--------------------------------------------------------------------------------------------------
    void writeData(const uint8_t *data, unsigned int length) override
    {
        if (length > OledWidth * OledPages)
            return;

        setDataPin();
        setChipSelect(true);

        HAL_SPI_Transmit_DMA(peripherie, const_cast<uint8_t *>(data), length);
        waitForTXComplete();

        setChipSelect(false);
    }

    //--------------------------------------------------------------------------------------------------
private:
    SPI_HandleTypeDef *peripherie = nullptr;

    void setDataPin()
    {
        HAL_GPIO_WritePin(DisplayDC_GPIO_Port, DisplayDC_Pin, GPIO_PIN_SET);
    }

    //--------------------------------------------------------------------------------------------------
    void setCommandPin()
    {
        HAL_GPIO_WritePin(DisplayDC_GPIO_Port, DisplayDC_Pin, GPIO_PIN_RESET);
    }

    //--------------------------------------------------------------------------------------------------
    void setChipSelect(bool state)
    {
        HAL_GPIO_WritePin(DisplayCS_GPIO_Port, DisplayCS_Pin,
                          state ? GPIO_PIN_RESET : GPIO_PIN_SET);
    }

    void waitForTXComplete()
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
};
