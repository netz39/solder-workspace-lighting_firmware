TARGET	:= solder_workspace_lighting
RTOS    := freertos
DEVICE	:= stm32l01d3

INCDIRS			:=  \
src 				\

SOURCES			:=  \
src/main.cxx

# Actual build engine
include core/mk/include.mk