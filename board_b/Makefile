# https://yuukidach.github.io/p/makefile-for-projects-with-subdirectories/

################################################################################
# SOURCE FILES
################################################################################

CMSIS_SUBDIR = STM32CubeH7/Drivers/CMSIS
CMSIS_DEVICE_SUBDIR = $(CMSIS_SUBDIR)/Device/ST/STM32H7xx
STARTUP_OBJ = $(CMSIS_DEVICE_SUBDIR)/Source/Templates/gcc/startup_stm32h750xx.o

HAL_SUBDIR = STM32CubeH7/Drivers/STM32H7xx_HAL_Driver
HAL_SOURCE = $(wildcard $(HAL_SUBDIR)/Src/*.c)
HAL_OBJS = $(HAL_SOURCE:c=o)

APP_SOURCE = $(wildcard *.c)
APP_OBJS = $(APP_SOURCE:c=o)

OBJS = $(STARTUP_OBJ) $(HAL_OBJS) $(APP_OBJS)

# Sourced from STM32CubeH7/Projects/STM32H750B-DK/Examples/PWR/PWR_STOP_STANDBY/STM32CubeIDE/STM32H750XBHX_FLASH.ld
LDSCRIPT = STM32H750XBHX_FLASH.ld
################################################################################


################################################################################
# COMPILER FLAGS
################################################################################
#DEFINES = -D"USE_HAL_DRIVER" -D"STM32F401xC" -D"USE_STM32F4XX_NUCLEO_144" -D"STM32F4xx=1"
DEFINES = -D"USE_HAL_DRIVER" -D"STM32H750xx" -D"APPLICATION_ADDRESS=0x08000000U" -D"USE_PWR_LDO_SUPPLY"
INCLUDE = -I"include/" -I"$(CMSIS_SUBDIR)/Core/Include" -I"$(CMSIS_DEVICE_SUBDIR)/Include" -I"$(HAL_SUBDIR)/Inc"

# https://github.com/MayaPosch/Nodate/blob/master/arch/stm32/Makefile
MCU_FLAGS := -mcpu=cortex-m7 -mfloat-abi=hard

# -mfpu=fpv4-sp-d16

CC	 = arm-none-eabi-gcc
FLAGS	 = $(MCU_FLAGS) $(DEFINES) $(INCLUDE) -g -c -O2 -Wall -Wextra
LFLAGS	 = $(MCU_FLAGS) -T $(LDSCRIPT) -Wl,--print-memory-usage -Wl,--gc-sections -Wl,-Map=firmware.map,--cref
################################################################################


################################################################################
# RULES
################################################################################
all: firmware.elf
	@echo "Done"

firmware.elf: $(OBJS)
	@echo " [LD] firmware.elf"
	@$(CC) -g $^ -o firmware.elf $(LFLAGS)

# All .c files
%.o: %.c
	@echo " [CC]" $<
	@$(CC) $(FLAGS) -o $@ $<

# Except template files
%_template.o: %_template.c
	@echo " [IGN]" $<
	@touch $@

# Startup
%.o: %.s
	@echo " [AS]" $<
	@$(CC) $(FLAGS) -o $@ $<

flash: firmware.elf
	arm-none-eabi-objcopy -O binary firmware.elf firmware.bin
	python3 -m stm32loader -p /dev/ttyACM0 -e -v -w firmware.bin
	python3 -m stm32loader -p /dev/ttyACM0 -g 0x08000000

clean:
	@rm -f $(OBJS) firmware.elf firmware.map
################################################################################
