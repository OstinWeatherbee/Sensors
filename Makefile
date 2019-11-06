ifeq ($(OS),Windows_NT) 
    detected_OS := Windows
else
    detected_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown')
endif

CC=arm-none-eabi-gcc
WARNING=-Wall
OBJECT_DIR=out
#OBJS := $(addprefix $(OBJECT_DIR)/,main.o )
OBJS := main.c

CFLAGS=-I -std=c99 -O2 -Xlinker \
	-Map=$(OBJECT_DIR)/output.map \
	--specs=nosys.specs



#$(OBJECT_DIR)/%.o: %.c
#	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJECT_DIR)/main: $(OBJS)| $(OBJECT_DIR)
	$(CC) -o $@ $^ $(CFLAGS) $(WARNING)

#$(OBJS): | $(OBJECT_DIR)

$(OBJECT_DIR):
	mkdir $@


.PHONY: clean

clean:
ifeq ($(detected_OS),Windows) 
	rmdir /s /q $(OBJECT_DIR)
else
	rm -r $(OBJECT_DIR)
endif
