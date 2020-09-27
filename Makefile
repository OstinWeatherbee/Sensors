ifeq ($(OS),Windows_NT) 
    detected_OS := Windows
else
    detected_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown')
endif

TARGET	= sensors
WARNING	= -Wall
DEFINES += STM32F103
MCU += -mcpu=cortex-m3

include sensors.mk

#Toolchain
#-------------------------------------------------------------------------------
AS = arm-none-eabi-as
CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
CP = arm-none-eabi-objcopy
SZ = arm-none-eabi-size
DMP = arm-none-eabi-objdump
SF = st-link_cli
#-------------------------------------------------------------------------------

#GCC config
#-------------------------------------------------------------------------------
CFLAGS += -mthumb -mthumb-interwork -mcpu=cortex-m3 -mlittle-endian
CFLAGS += -I -std=c99 -O2 -Wall
CFLAGS += -ggdb
CFLAGS += $(addprefix -I, $(INCLUDES))
CFLAGS += $(addprefix -D, $(DEFINES))
#-------------------------------------------------------------------------------


#Linker script
#-------------------------------------------------------------------------------
LDSCRIPT   = STM32F103XB_FLASH.ld
#-------------------------------------------------------------------------------
 
#Linker config
#-------------------------------------------------------------------------------
LDFLAGS += -mthumb -mthumb-interwork -mcpu=cortex-m3 -mlittle-endian
LDFLAGS += -nostartfiles  -nostdlib
LDFLAGS += -T $(LDSCRIPT)	#use linker script (aka scatter for ARM compiler)
LDFLAGS += --specs=nosys.specs
LDFLAGS += -Xlinker -Map=$(OBJ_DIR)/output.map
#-------------------------------------------------------------------------------

#ASM config
#-------------------------------------------------------------------------------
#AFLAGS += -Wnls -mapcs
AFLAGS += -mapcs-32
AFLAGS += $(addprefix -I, $(INCLUDES))
#-------------------------------------------------------------------------------

all: $(OBJ_DIR)/$(TARGET).elf

$(OBJ_DIR)/$(TARGET).elf: $(OBJS)
	@echo Linking		$@
	@$(LD) $(LDFLAGS) $^ -o $@ -lm
	@echo Create binary	$(OBJ_DIR)/$(TARGET).bin
	@$(CP) -O binary -I elf32-littlearm $@ $(OBJ_DIR)/$(TARGET).bin
	@echo Create hex	$(OBJ_DIR)/$(TARGET).hex
	@$(CP) -O ihex $@ $(OBJ_DIR)/$(TARGET).hex
	@echo Create assembly intermixed with sources	$(OBJ_DIR)/$(TARGET).s
	@$(DMP) -SD $@ > $(OBJ_DIR)/$(TARGET).s

#Compile Obj files from C
#-------------------------------------------------------------------------------
$(OBJ_DIR)/%.o: %.c
	@echo Compiling	$<
	@$(CC) $(CFLAGS) -MD -c $< -o $@
#-------------------------------------------------------------------------------
 
#Compile Obj files from asm
#-------------------------------------------------------------------------------
$(OBJ_DIR)/%.o: %.s
	@echo Compiling	$<
	@$(AS) $(AFLAGS) -c $< -o $@
#-------------------------------------------------------------------------------

$(OBJS): | $(OBJ_DIR)

COMMA := ,
EMPTY :=
SPACE := $(EMPTY) $(EMPTY)

#Create all output directories
$(OBJ_DIR): $(SRC)
	mkdir $@
	@for /D %%i IN ($(strip $(subst /,\,$(subst $(SPACE),$(COMMA),$(addprefix $@/,$^))))) DO (mkdir %%i)


.PHONY: clean
clean:
ifeq ($(detected_OS),Windows) 
	rmdir /s /q $(OBJ_DIR)
else
	rm -r $(OBJ_DIR)
endif

.PHONY: flash
flash:
#Full erase
	$(SF) -ME
#Program
	$(SF) -P $(OBJ_DIR)/$(TARGET).bin 0x8000000
#Verify
	$(SF) -V