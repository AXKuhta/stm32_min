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

# Debugging ang flashing
#
# SWD doesn't work? Try:
# - Adding 100 ohm resistor between SWDIO and GND
# - Twisting SWD wires with a GND wire
#
# Alternatively, tie BOOT0 high and flash via bootrom uart:
# arm-none-eabi-objcopy -O binary firmware.elf firmware.bin
# python3 -m stm32loader -p /dev/ttyACM0 -e -v -w firmware.bin
# python3 -m stm32loader -p /dev/ttyACM0 -g 0x08000000
#
# https://www.st.com/resource/en/application_note/an2606-stm32-microcontroller-system-memory-boot-mode-stmicroelectronics.pdf
debug:
	@openocd -f interface/cmsis-dap.cfg -f target/stm32h7x.cfg & gdb-multiarch ./firmware.elf -ex "target extended-remote :3333" -ex "monitor [target current] configure -event gdb-detach {shutdown}"

flash: firmware.elf
	@openocd -f interface/cmsis-dap.cfg -f target/stm32h7x.cfg -c "program firmware.elf verify reset exit"

clean:
	@rm -f $(OBJS) firmware.elf firmware.map
################################################################################
