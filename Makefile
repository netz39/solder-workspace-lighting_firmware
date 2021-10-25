TARGET  := solder_workspace_lighting
RTOS    := freertos
DEVICE  := stm32l432

INCDIRS :=              \
src/oled-driver/include \
src/oled-driver/        \
src

SOURCES :=                          \
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