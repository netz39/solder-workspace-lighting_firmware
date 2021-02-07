ifeq ($(RELEASE),1)
CONFIGURATION	:= release
RELEASE_OPT		?= 3
OPTIMIZATION	:= -O$(RELEASE_OPT) -g
else
CONFIGURATION	:= debug
OPTIMIZATION	:= -Og -g
DEFS			+= DEBUG
endif

ROOTDIR		:= $(dir $(firstword $(MAKEFILE_LIST)))
BASEDIR		:= $(dir $(lastword $(MAKEFILE_LIST)))../
MXDIR		:= $(ROOTDIR)cubemx/
OUTPUT_DIR	:= $(ROOTDIR)build/$(CONFIGURATION)/
OBJDIR		:= $(OUTPUT_DIR)/obj

INCDIRS		+= \
$(BASEDIR)include \

SOURCES		+= \
$(BASEDIR)src/abi.cpp \
$(BASEDIR)src/std.cpp

# CubeMX
include cubemx/Makefile

SOURCES += $(foreach source,$(C_SOURCES) $(ASM_SOURCES),$(MXDIR)$(source))
INCDIRS += $(C_INCLUDES:-I%=$(MXDIR)%)
INCDIRS += $(AS_INCLUDES:-I%=$(MXDIR)%)

LDSCRIPT := $(MXDIR)$(LDSCRIPT)

# Include actual rules
include $(BASEDIR)mk/rules.mk
