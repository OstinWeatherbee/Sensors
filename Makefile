CC=arm-none-eabi-gcc
WARNING=-Wall
OBJECT_DIR=out
#OBJS := $(addprefix $(OBJECT_DIR)/,main.o )
OBJS := main.c

CFLAGS=-I -std=c99 -O2 -Xlinker -Map=$(OBJECT_DIR)/output.map 



#$(OBJECT_DIR)/%.o: %.c
#	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJECT_DIR)/main: $(OBJS)| $(OBJECT_DIR)
	$(CC) -o $@ $^ $(CFLAGS) $(WARNING)

#$(OBJS): | $(OBJECT_DIR)

$(OBJECT_DIR):
	mkdir $@

clean:
	rmdir /s /q $(OBJECT_DIR)
