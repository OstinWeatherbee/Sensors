ifeq ($(OS),Windows_NT) 
    detected_OS := Windows
else
    detected_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown')
endif

TARGET	= sensors
WARNING	= -Wall
OBJECT_DIR	= out
DEFINES = STM32F103
MCU += -mcpu=cortex-m3

#Source path
#-------------------------------------------------------------------------------
SOURCEDIRS := src
#-------------------------------------------------------------------------------

#Header path
#-------------------------------------------------------------------------------
INCLUDES += inc
#-------------------------------------------------------------------------------

#Toolchain
#-------------------------------------------------------------------------------
AS = arm-none-eabi-gcc
CC = arm-none-eabi-gcc
LD = arm-none-eabi-g++
CP = arm-none-eabi-objcopy
SZ = arm-none-eabi-size
#-------------------------------------------------------------------------------

#GCC config
#-------------------------------------------------------------------------------
CFLAGS=-I -std=c99 -O2 -Xlinker \
	-Map=$(OBJECT_DIR)/output.map 
CFLAGS += $(addprefix -I, $(INCLUDES))
CFLAGS += $(addprefix -D, $(DEFINES))
#-------------------------------------------------------------------------------


#Linker script
#-------------------------------------------------------------------------------
LDSCRIPT   = gcc_arm.ld
#-------------------------------------------------------------------------------
 
#Linker config
#-------------------------------------------------------------------------------
LDFLAGS += -nostartfiles  -nostdlib -mthumb $(MCU)
LDFLAGS += -T $(LDSCRIPT)
LDFLAGS += --specs=nosys.specs
#-------------------------------------------------------------------------------

#ASM config
#-------------------------------------------------------------------------------
AFLAGS += -Wnls -mapcs
#-------------------------------------------------------------------------------

#Obj file list
#-------------------------------------------------------------------------------
OBJS += $(patsubst %.c, %.o, $(wildcard  $(addsuffix /*.c, $(SOURCEDIRS))))
OBJS += $(patsubst %.s, %.o, $(wildcard  $(addsuffix /*.s, $(SOURCEDIRS))))
#-------------------------------------------------------------------------------

all: $(TARGET).hex $(TARGET).bin $(TARGET).elf

$(TARGET).hex: $(TARGET).elf
	@$(CP) -O ihex $(TARGET).elf $(TARGET).hex

$(TARGET).bin: $(TARGET).elf
	@$(CP) -O binary $(TARGET).elf $(TARGET).bin

$(TARGET).elf: $(OBJS)
	@$(LD) $(LDFLAGS) $^ -o $@


#Compile Obj files from C
#-------------------------------------------------------------------------------
%.o: %.c
	@$(CC) $(CFLAGS) -MD -c $< -o $@
#-------------------------------------------------------------------------------
 
#Compile Obj files from asm
#-------------------------------------------------------------------------------
%.o: %.s
	@$(AS) $(AFLAGS) -c $< -o $@
#-------------------------------------------------------------------------------

# $(OBJECT_DIR)/main: $(OBJS)| $(OBJECT_DIR)
# 	$(CC) -o $@ $^ $(CFLAGS) $(WARNING)

# #$(OBJS): | $(OBJECT_DIR)

# $(OBJECT_DIR):
# 	mkdir $@


.PHONY: clean

clean:
ifeq ($(detected_OS),Windows) 
	rmdir /s /q $(OBJECT_DIR)
else
	rm -r $(OBJECT_DIR)
endif
