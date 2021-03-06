PROJECT_NAME := wearlink

MAKEFILE_NAME := $(MAKEFILE_LIST)
MAKEFILE_DIR := $(dir $(MAKEFILE_NAME))

SDK_PATH = ./nrf51_sdk_10_0_0
PROJECT_PATH = .
TEMPLATE_PATH = $(SDK_PATH)/components/toolchain/gcc
GNU_INSTALL_ROOT := /usr
GNU_VERSION := 4.9.3
GNU_PREFIX := arm-none-eabi

OUTPUT_FILENAME := nrf51822_xxaa_s110
SOFTDEVICE_FILENAME := $(SDK_PATH)/components/softdevice/s110/hex/s110_nrf51_8.0.0_softdevice.hex
export OUTPUT_FILENAME

MK := mkdir
RM := rm -rf

#echo suspend
ifeq ("$(VERBOSE)","1")
NO_ECHO :=
else
NO_ECHO := @
endif

# Toolchain commands
CC              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-gcc'
GDB             := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-gdb'
AS              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-as'
AR              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ar' -r
LD              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ld'
NM              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-nm'
OBJDUMP         := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objdump'
OBJCOPY         := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objcopy'
SIZE            := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-size'

#function for removing duplicates in a list
remduplicates = $(strip $(if $1,$(firstword $1) $(call remduplicates,$(filter-out $(firstword $1),$1))))

#source common to all targets
C_SOURCE_FILES += \
$(SDK_PATH)/components/libraries/button/app_button.c \
$(SDK_PATH)/components/libraries/util/app_error.c \
$(SDK_PATH)/components/libraries/fifo/app_fifo.c \
$(SDK_PATH)/components/libraries/scheduler/app_scheduler.c \
$(SDK_PATH)/components/libraries/timer/app_timer.c \
$(SDK_PATH)/components/libraries/timer/app_timer_appsh.c \
$(SDK_PATH)/components/libraries/trace/app_trace.c \
$(SDK_PATH)/components/libraries/util/nrf_assert.c \
$(SDK_PATH)/components/libraries/uart/retarget.c \
$(SDK_PATH)/components/libraries/uart/app_uart_fifo.c \
$(SDK_PATH)/components/drivers_nrf/delay/nrf_delay.c \
$(SDK_PATH)/components/drivers_nrf/common/nrf_drv_common.c \
$(SDK_PATH)/components/drivers_nrf/gpiote/nrf_drv_gpiote.c \
$(SDK_PATH)/components/drivers_nrf/uart/nrf_drv_uart.c \
$(SDK_PATH)/components/drivers_nrf/pstorage/pstorage.c \
$(SDK_PATH)/components/drivers_nrf/hal/nrf_adc.c \
$(PROJECT_PATH)/main.c \
$(SDK_PATH)/components/ble/common/ble_advdata.c \
$(SDK_PATH)/components/ble/ble_advertising/ble_advertising.c \
$(SDK_PATH)/components/ble/ble_services/ble_bas/ble_bas.c \
$(SDK_PATH)/components/ble/common/ble_conn_params.c \
$(SDK_PATH)/components/ble/ble_services/ble_dis/ble_dis.c \
$(SDK_PATH)/components/ble/ble_services/ble_hids/ble_hids.c \
$(SDK_PATH)/components/ble/common/ble_srv_common.c \
$(SDK_PATH)/components/ble/device_manager/device_manager_peripheral.c \
$(SDK_PATH)/components/toolchain/system_nrf51.c \
$(SDK_PATH)/components/softdevice/common/softdevice_handler/softdevice_handler.c \
$(SDK_PATH)/components/softdevice/common/softdevice_handler/softdevice_handler_appsh.c \
$(SDK_PATH)/examples/bsp/bsp.c

#assembly files common to all targets
ASM_SOURCE_FILES  = $(SDK_PATH)/components/toolchain/gcc/gcc_startup_nrf51.s

# Select board
#INC_PATHS  = -I$(PROJECT_PATH)/config/wearlink_s110_ble400
INC_PATHS  = -I$(PROJECT_PATH)/config/wearlink_s110_s4at

#includes common to all targets
INC_PATHS += -I$(PROJECT_PATH)/config
INC_PATHS += -I$(PROJECT_PATH)/
INC_PATHS += -I$(SDK_PATH)/components/libraries/scheduler
INC_PATHS += -I$(SDK_PATH)/components/drivers_nrf/config
INC_PATHS += -I$(SDK_PATH)/examples/bsp
INC_PATHS += -I$(SDK_PATH)/components/libraries/fifo
INC_PATHS += -I$(SDK_PATH)/components/ble/ble_services/ble_hids
INC_PATHS += -I$(SDK_PATH)/components/drivers_nrf/delay
INC_PATHS += -I$(SDK_PATH)/components/libraries/util
INC_PATHS += -I$(SDK_PATH)/components/ble/device_manager
INC_PATHS += -I$(SDK_PATH)/components/drivers_nrf/uart
INC_PATHS += -I$(SDK_PATH)/components/ble/common
INC_PATHS += -I$(SDK_PATH)/components/drivers_nrf/pstorage
INC_PATHS += -I$(SDK_PATH)/components/ble/ble_services/ble_dis
INC_PATHS += -I$(SDK_PATH)/components/device
INC_PATHS += -I$(SDK_PATH)/components/libraries/uart
INC_PATHS += -I$(SDK_PATH)/components/libraries/button
INC_PATHS += -I$(SDK_PATH)/components/libraries/timer
INC_PATHS += -I$(SDK_PATH)/components/softdevice/s110/headers
INC_PATHS += -I$(SDK_PATH)/components/drivers_nrf/gpiote
INC_PATHS += -I$(SDK_PATH)/components/drivers_nrf/hal
INC_PATHS += -I$(SDK_PATH)/components/toolchain/gcc
INC_PATHS += -I$(SDK_PATH)/components/toolchain
INC_PATHS += -I$(SDK_PATH)/components/drivers_nrf/common
INC_PATHS += -I$(SDK_PATH)/components/ble/ble_advertising
INC_PATHS += -I$(SDK_PATH)/components/libraries/trace
INC_PATHS += -I$(SDK_PATH)/components/ble/ble_services/ble_bas
INC_PATHS += -I$(SDK_PATH)/components/softdevice/common/softdevice_handler

OBJECT_DIRECTORY = _build
LISTING_DIRECTORY = $(OBJECT_DIRECTORY)
OUTPUT_BINARY_DIRECTORY = $(OBJECT_DIRECTORY)

# Sorting removes duplicates
BUILD_DIRECTORIES := $(sort $(OBJECT_DIRECTORY) $(OUTPUT_BINARY_DIRECTORY) $(LISTING_DIRECTORY) )

#flags common to all targets
CFLAGS  = -DBOARD_CUSTOM
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DNRF51
CFLAGS += -DS110
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -DSWI_DISABLE0

# Only for BLE400 board
# CFLAGS += -DBSP_UART_SUPPORT
# CFLAGS += -DENABLE_DEBUG_LOG_SUPPORT

CFLAGS += -DDEBUG
CFLAGS += -Wall -g -O0
CFLAGS += -mcpu=cortex-m0
CFLAGS += -mthumb -mabi=aapcs --std=gnu99
CFLAGS += -mfloat-abi=soft
# keep every function in separate section. This will allow linker to dump unused functions
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums

# keep every function in separate section. This will allow linker to dump unused functions
LDFLAGS += -Xlinker -Map=$(LISTING_DIRECTORY)/$(basename $(notdir $(LINKER_SCRIPT))).map
LDFLAGS += -mthumb -mabi=aapcs -L $(TEMPLATE_PATH) -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m0
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs -lc -lnosys

# Assembler flags
ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -DBOARD_CUSTOM
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DNRF51
ASMFLAGS += -DS110
ASMFLAGS += -DBLE_STACK_SUPPORT_REQD
ASMFLAGS += -DSWI_DISABLE0
ASMFLAGS += -DBSP_UART_SUPPORT
#default target - first one defined
default: clean nrf51822_xxac_s110 nrf51822_xxaa_s110 finalize

#building all targets
all: clean
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e cleanobj
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e nrf51822_xxac_s110
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e nrf51822_xxaa_s110

#target for printing all targets
help:
	@echo following targets are available:
	@echo 	nrf51822_xxac_s110
	@echo 	nrf51822_xxaa_s110
	@echo 	flash_softdevice


C_SOURCE_FILE_NAMES = $(notdir $(C_SOURCE_FILES))
C_PATHS = $(call remduplicates, $(dir $(C_SOURCE_FILES) ) )
C_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(C_SOURCE_FILE_NAMES:.c=.o) )
D_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(C_SOURCE_FILE_NAMES:.c=.d) )

ASM_SOURCE_FILE_NAMES = $(notdir $(ASM_SOURCE_FILES))
ASM_PATHS = $(call remduplicates, $(dir $(ASM_SOURCE_FILES) ))
ASM_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(ASM_SOURCE_FILE_NAMES:.s=.o) )

vpath %.c $(C_PATHS)
vpath %.s $(ASM_PATHS)

OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

nrf51822_xxac_s110: LINKER_SCRIPT=config/wearlink_s110_ble400/nrf51822_xxac_s110.ld
nrf51822_xxac_s110: $(BUILD_DIRECTORIES) $(OBJECTS) $(D_OBJECTS)
	@echo Linking target: $(OUTPUT_BINARY_DIRECTORY)/$(basename $(notdir $(LINKER_SCRIPT))).out
	$(NO_ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(OUTPUT_BINARY_DIRECTORY)/$(basename $(notdir $(LINKER_SCRIPT))).out

nrf51822_xxaa_s110: LINKER_SCRIPT=config/wearlink_s110_s4at/nrf51822_xxaa_s110.ld
nrf51822_xxaa_s110: $(BUILD_DIRECTORIES) $(OBJECTS) $(D_OBJECTS)
	@echo Linking target: $(OUTPUT_BINARY_DIRECTORY)/$(basename $(notdir $(LINKER_SCRIPT))).out
	$(NO_ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(OUTPUT_BINARY_DIRECTORY)/$(basename $(notdir $(LINKER_SCRIPT))).out

## Create build directories
$(BUILD_DIRECTORIES):
	echo $(MAKEFILE_NAME)
	$(MK) $@

# Create objects from C SRC files
$(OBJECT_DIRECTORY)/%.o: %.c
	@echo Compiling file: $(notdir $<)
	$(NO_ECHO)$(CC) $(CFLAGS) $(INC_PATHS) -c -o $@ $<

$(OBJECT_DIRECTORY)/%.d: %.c
	@echo Dumping defines for file: $(notdir $<)
	$(NO_ECHO)$(CC) $(CFLAGS) $(INC_PATHS) -c -dM -E $< | sort > $@

# Assemble files
$(OBJECT_DIRECTORY)/%.o: %.s
	@echo Compiling file: $(notdir $<)
	$(NO_ECHO)$(CC) $(ASMFLAGS) $(INC_PATHS) -c -o $@ $<


# Link
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out: $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo Linking target: $(OUTPUT_FILENAME).out
	$(NO_ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out


## Create binary .bin file from the .out file
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	@echo Preparing: $(OUTPUT_FILENAME).bin
	$(NO_ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin

## Create binary .hex file from the .out file
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	@echo Preparing: $(OUTPUT_FILENAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex

finalize: genbin genhex echosize

genbin:
	@echo Preparing: $(OUTPUT_FILENAME).bin
	$(NO_ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin

## Create binary .hex file from the .out file
genhex:
	@echo Preparing: $(OUTPUT_FILENAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex

echosize:
	-@echo ''
	$(NO_ECHO)$(SIZE) $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	-@echo ''

clean:
	$(RM) $(BUILD_DIRECTORIES)

cleanobj:
	$(RM) $(BUILD_DIRECTORIES)/*.o

flash:
	@echo Flashing: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex
	openocd -f openocd-cli.cfg -c "init; program $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex verify; exit"

flash_softdevice:
	@echo Flashing: $(SOFTDEVICE_FILENAME)
	openocd -f openocd-cli.cfg -c "init; program $(SOFTDEVICE_FILENAME) verify; exit"

concat:
	srec_cat $(SOFTDEVICE_FILENAME) -intel $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex -intel -o $(OUTPUT_BINARY_DIRECTORY)/wearlink_s110.hex -intel --line-length=44

flash_all:
	@echo Flashing: all
	openocd -f openocd-cli.cfg -c "init; program $(OUTPUT_BINARY_DIRECTORY)/wearlink_s110.hex verify; exit"

.gdb_config_oocd:
	echo -e > .gdb_config_oocd "file $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out\ntarget extended-remote | openocd -f openocd.cfg\n"

debug: .gdb_config_oocd
	$(GDB) --command=.gdb_config_oocd

mass_erase:
	# For nrf51822-S4AT modules, that come preinstalled with factory firmware
	openocd -f openocd-cli.cfg -c "init; nrf51 mass_erase"
