#
# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#   2017-08-14 - octopusy changed..
#	2017-02-10 - Several enhancements + project update mode
#   2015-07-22 - first version
# ------------------------------------------------

######################################
# target
######################################
TARGET = main

######################################
# BUILD version
# format :  [ major.minor.date ]
######################################
BUILD_VER = "01"

BUILD_DATE = "$(shell date +'%Y-%m-%d')"

######################################
# user bootloader
######################################
USE_IAP = yes

######################################
# building variables
######################################
# debug build?
DEBUG = 0
# optimization
OPT = -O2 -Os


######################################
# Test Mode
######################################
USED_TEST_ONLY = no

######################################
# Support all Language
######################################
USED_ALL_LANGUAGE = no

######################################
# JAPAN Language
######################################
USED_JAPAPN_MODE = no

######################################
# WORD LIST Big font
######################################
USED_BIG_FONT = yes

######################################
# Code Optimization
######################################
USED_CODE_OPT = yes

######################################
# Used Passphrase
######################################
USED_PASSPHRASE = yes

USED_BOOT_HW_TEST = yes

#######################################
# paths
#######################################
# source path
SOURCES_DIR =

# firmware library path
PERIFLIB_PATH = 

# Build path
BUILD_DIR = build

# Build path
DEP_DIR = dep

######################################
# source
######################################
# C sources
C_SRC_PATH =  \
Drivers/STM32L0xx_HAL_Driver/Src \
Middlewares/ST/STM32_USB_Device_Library/Core/Src \
Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Src \
crypto/Src \
Src

C_SOURCES  := $(foreach path, $(C_SRC_PATH),$(shell find $(path) | grep "\.c$$") )

# ASM sources
ASM_SOURCES =  \
startup/startup_stm32l072xx.s


######################################
# firmware library
######################################
PERIFLIB_SOURCES = 


#######################################
# binaries
#######################################
OS := $(shell uname -o)
ifeq ($(OS),Cygwin)
   BINPATH="c:/cygwin/5.4 2016q3/bin"
else
   BINPATH="/opt/toolchains/gcc-arm-none-eabi-5_4-2016q3/bin"
endif

PREFIX = arm-none-eabi-
CC = $(BINPATH)/$(PREFIX)gcc
AS = $(BINPATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(BINPATH)/$(PREFIX)objcopy
AR = $(BINPATH)/$(PREFIX)ar
SZ = $(BINPATH)/$(PREFIX)size
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m0plus

# fpu
# NONE for Cortex-M0/M0+/M3

# float-abi


# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32L072xx


# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-IInc \
-Icrypto/Inc \
-IDrivers/STM32L0xx_HAL_Driver/Inc \
-IDrivers/STM32L0xx_HAL_Driver/Inc/Legacy \
-IMiddlewares/ST/STM32_USB_Device_Library/Core/Inc \
-IMiddlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Inc \
-IDrivers/CMSIS/Device/ST/STM32L0xx/Include \
-IDrivers/CMSIS/Include


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

ifneq ($(BUILD_VER), )
C_DEFS += -DBUILD_VER='$(BUILD_VER)' -DBUILD_DATE='$(BUILD_DATE)'
endif

ifeq ($(USE_IAP), yes)
C_DEFS += -DD_USER_BOOTLOADER
endif

#C_DEFS += -DFEATURE_USB_CDC_PORTING

ifeq ($(USED_TEST_ONLY), yes)
C_DEFS += -DD_TEST_ONLY
endif

ifeq ($(USED_ALL_LANGUAGE), yes)
C_DEFS += -DD_ALL_LANGUAGE
else
ifeq ($(USED_JAPAPN_MODE), yes)
C_DEFS += -DD_JAPAN_MODE
endif
endif

ifeq ($(USED_BIG_FONT), yes)
C_DEFS += -DD_WORD_BIG_FONT
endif

ifeq ($(USED_CODE_OPT), yes)
C_DEFS += -DD_CODE_OPTIMIZE
endif

ifeq ($(USED_PASSPHRASE), yes)
C_DEFS += -DD_ADD_PASSPHRASE_IN_SEED
endif

ifeq ($(USED_BOOT_HW_TEST), yes)
C_DEFS += -DD_BOOT_HW_TEST
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
ifeq ($(USE_IAP), yes)
LDSCRIPT = hkassemain.ld
else
LDSCRIPT = hkasse.ld
endif

# libraries
LIBS = -lc -lm -lnosys
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

#######################################
# dependencies
#######################################
DEPEND_FILES := $(sort $(addprefix $(DEP_DIR)/, $(addsuffix .d, $(basename $(notdir $(C_SOURCES))))))
-include $(DEPEND_FILES)

$(DEP_DIR)/%.d: %.c Makefile
	@echo "dependency $@"
	@mkdir -p dep
	$(CC) -M $(CFLAGS) $< | sed 's/\($*\)\.o[ :]*/obj\/\1.o: /g' | sed -e 's/[:\t ][^ ]\+\.c//g' > $(DEP_DIR)/$(basename $(notdir $@)).d 2>/dev/null

$(BUILD_DIR)/%.o: %.c Makefile $(DEP_DIR)/%.d | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@echo 
	@echo "****************************************************"
	@$(SZ) $@
	@echo "****************************************************"
	@echo 

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(DEP_DIR) $(BUILD_DIR) build.log
  

# *** EOF ***
