LINKER_SCRIPT = modified_default.ld

CFLAGS = -g3
# CFLAGS = -O3
# CFLAGS += -fsanitize=address
CFLAGS += -fno-omit-frame-pointer
CFLAGS += -T $(LINKER_SCRIPT)

INCLUDE_DIR = include
SOURCE_DIR = src

SOURCES := $(shell find $(SOURCE_DIR) -name "*.c")
INCLUDES := $(shell find $(INCLUDE_DIR) -name "*.h")

LIB_SOURCES = $(shell find $(LIB_SOURCE_DIR) -name "*.c")
LIB_INCLUDES = $(shell find $(LIB_INCLUDE_DIR) -name "*.h")

build/ukernel_lisp.out: $(SOURCES) $(INCLUDES) $(LIB_SOURCES) $(LIB_INCLUDES)
	gcc $(CFLAGS) -Iinclude $^ -o $@
