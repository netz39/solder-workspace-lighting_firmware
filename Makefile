TARGET  := solder_workspace_lighting
DEVICE  := stm32l432

DEFS    += FW_USE_RTOS

INCDIRS :=              \
src/oled-driver/include \
src/oled-driver/        \
src

SOURCES :=                          \
src/Button/Button.cxx               \
src/Button/buttonHandler.cxx        \
src/adc.cxx         				\
src/oled-driver/fonts/mono.cxx      \
src/oled-driver/src/Display.cxx     \
src/oled-driver/src/Image.cxx       \
src/oled-driver/src/Renderer.cxx    \
src/encoder.cxx                     \
src/leds.cxx                        \
src/OledDisplay.cxx

# Actual build engine
include core/mk/include.mk