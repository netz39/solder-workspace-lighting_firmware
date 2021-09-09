TARGET  := solder_workspace_lighting
RTOS    := freertos
DEVICE  := stm32l432

INCDIRS :=          \
src                 \

SOURCES :=          \
src/adc.cxx         \
src/encoder.cxx     \
src/leds.cxx        \
src/main.cxx

# Actual build engine
include core/mk/include.mk