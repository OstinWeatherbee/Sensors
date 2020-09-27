#Header path
#-------------------------------------------------------------------------------
INCLUDES += inc
INCLUDES += src
INCLUDES += src/driver
INCLUDES += src/FreeRTOS/Source/include
INCLUDES += src/FreeRTOS/Source/portable
INCLUDES += src/utils
#-------------------------------------------------------------------------------

#Source files
#-------------------------------------------------------------------------------
SRC += \
	src \
	src/driver \
	src/FreeRTOS/Source \
	src/FreeRTOS/Source/portable \
	src/utils

SRC_EXCLUDE += \
	src/FreeRTOS/Source/portable/heap_1.c \
	src/FreeRTOS/Source/portable/heap_2.c \
	src/FreeRTOS/Source/portable/heap_3.c \
	src/FreeRTOS/Source/portable/heap_4.c \
	src/FreeRTOS/Source/portable/heap_5.c

SRC_C += $(wildcard  $(addsuffix /*.c, $(SRC)))
SRC_C := $(filter-out $(SRC_EXCLUDE), $(SRC_C))
SRC_S += $(wildcard  $(addsuffix /*.s, $(SRC)))
SRC_S := $(filter-out $(SRC_EXCLUDE), $(SRC_S))
#-------------------------------------------------------------------------------

OBJ_DIR	= out

#Obj file list
#-------------------------------------------------------------------------------
OBJS += $(patsubst %.c, %.o, $(addprefix $(OBJ_DIR)/, $(SRC_C)))
OBJS += $(patsubst %.s, %.o, $(addprefix $(OBJ_DIR)/, $(SRC_S)))
#-------------------------------------------------------------------------------