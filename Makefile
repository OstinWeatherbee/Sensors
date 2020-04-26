ifeq ($(OS),Windows_NT) 
    detected_OS := Windows
else
    detected_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown')
endif

TARGET	= sensors
WARNING	= -Wall
OBJ_DIR	= out
DEFINES = STM32F103
MCU += -mcpu=cortex-m3

#Source path
#-------------------------------------------------------------------------------
SOURCEDIRS += src
#-------------------------------------------------------------------------------

#Header path
#-------------------------------------------------------------------------------
INCLUDES += inc
#-------------------------------------------------------------------------------

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
LDFLAGS += -nostartfiles  -nostdlib
#LDFLAGS += -nostartfiles -mthumb $(MCU)
LDFLAGS += -T $(LDSCRIPT)	#use linker script (aka scatter for ARM compiler)
LDFLAGS += --specs=nosys.specs
LDFLAGS += -Xlinker -Map=$(OBJ_DIR)/output.map
#-------------------------------------------------------------------------------

#ASM config
#-------------------------------------------------------------------------------
#AFLAGS += -Wnls -mapcs
AFLAGS += -mapcs-32
AFLAGS += $(addprefix -I, $(INCLUDES))
AFLAGS += $(addprefix -D, $(DEFINES))
#-------------------------------------------------------------------------------

#Source files
#-------------------------------------------------------------------------------
SRC += $(wildcard  $(SOURCEDIRS)/*.c)
SRC += $(wildcard  $(SOURCEDIRS)/*.s)
#-------------------------------------------------------------------------------

#Obj file list
#-------------------------------------------------------------------------------
#OBJS += $(patsubst %.c, %.o, $(wildcard  $(addsuffix /*.c, $(SOURCEDIRS))))
#OBJS += $(patsubst %.s, %.o, $(wildcard  $(addsuffix /*.s, $(SOURCEDIRS))))
OBJS += $(SRC:$(SOURCEDIRS)/%.c=$(OBJ_DIR)/%.o)
#-------------------------------------------------------------------------------


all: $(OBJ_DIR)/$(TARGET).elf

$(OBJ_DIR)/$(TARGET).elf: $(OBJS)
	@echo Linking		$@
	@$(LD) $(LDFLAGS) $^ -o $@
	@echo Create binary	$(OBJ_DIR)/$(TARGET).bin
	@$(CP) -O binary -I elf32-littlearm $@ $(OBJ_DIR)/$(TARGET).bin
#$(CP) -O binary -I elf32-littlearm --change-section-address=.data=0x8000000 -S $< $@
	@echo Create hex	$(OBJ_DIR)/$(TARGET).hex
	@$(CP) -O ihex $@ $(OBJ_DIR)/$(TARGET).hex
	@echo Create assembly intermixed with sources	$(OBJ_DIR)/$(TARGET).s
	@$(DMP) -SD $@ > $(OBJ_DIR)/$(TARGET).s

#Compile Obj files from C
#-------------------------------------------------------------------------------
$(OBJ_DIR)/%.o: $(SOURCEDIRS)/%.c
	@echo Compiling	$<
	@$(CC) $(CFLAGS) -MD -c $< -o $@
#-------------------------------------------------------------------------------
 
#Compile Obj files from asm
#-------------------------------------------------------------------------------
$(OBJ_DIR)/%.o: $(SOURCEDIRS)/%.s
	@echo Compiling	$<
	@$(AS) $(AFLAGS) -c $< -o $@
#-------------------------------------------------------------------------------

# $(OBJ_DIR)/main: $(OBJS)| $(OBJ_DIR)
# 	$(CC) -o $@ $^ $(CFLAGS) $(WARNING)

$(OBJS): | $(OBJ_DIR)

$(OBJ_DIR):
	mkdir $@


.PHONY: clean

clean:
ifeq ($(detected_OS),Windows) 
	rmdir /s /q $(OBJ_DIR)
else
	rm -r $(OBJ_DIR)
endif

flash:
#Full erase
	$(SF) -ME
#Program
	$(SF) -P $(OBJ_DIR)/$(TARGET).bin 0x8000000
#Verify
	$(SF) -V