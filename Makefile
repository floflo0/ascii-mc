NAME := ascii-mc
VERSION := 0.0.0

BUILD_TYPE ?= debug
PLATFORM = linux-x86_64

ifeq ($(BUILD_TYPE), release)
else ifeq ($(BUILD_TYPE), debug)
else
$(error Invalid BUILD_TYPE. Expected 'debug' or 'release', but got: $(BUILD_TYPE))
endif

ifeq ($(PLATFORM), linux-x86_64)
else ifeq ($(PLATFORM), wasm)
else
$(error Invalid PLATFORM. Expected 'linux-x86_64', but got: $(PLATFORM))
endif

ifeq ($(PLATFORM), wasm)
	CC := clang
else
	CC := gcc
endif

LIBS := libevdev libsystemd
CFLAGS +=                                \
	-std=gnu23                           \
	-Wall                                \
	-Wextra                              \
	-Wcast-qual                          \
	-Wmissing-prototypes                 \
	-DVERSION=\"$(VERSION)\"             \
	$(shell pkg-config --cflags $(LIBS)) \
	$(EXTRA_CFLAGS)
ifeq ($(BUILD_TYPE), release)
	CFLAGS += -DPROD -DNDEBUG -O3 -ffast-math -flto=auto -march=native
else ifeq ($(BUILD_TYPE), debug)
	CFLAGS += -ggdb
endif

LDFLAGS := $(shell pkg-config --libs $(LIBS)) -lm -lpthread
SRC_DIR := src
BASE_BUILD_DIR := build
BUILD_DIR := $(BASE_BUILD_DIR)/$(PLATFORM)/$(BUILD_TYPE)
ASSETS_DIR := assets
OBJS := $(patsubst    \
	$(SRC_DIR)/%.c,   \
	$(BUILD_DIR)/%.o, \
	$(filter-out $(SRC_DIR)/controller-wasm.c, $(wildcard $(SRC_DIR)/*.c)))
TEXTURE_C_FILES := $(patsubst $(ASSETS_DIR)/%.ppm,$(BUILD_DIR)/%.c,$(wildcard $(ASSETS_DIR)/*.ppm))
TEXTURE_OBJS := $(patsubst $(ASSETS_DIR)/%.ppm,$(BUILD_DIR)/%.o,$(wildcard $(ASSETS_DIR)/*.ppm))
EXEC := $(BUILD_DIR)/$(NAME)

WASM_CC := clang
WASM_BUILD_DIR := $(BASE_BUILD_DIR)/wasm/$(BUILD_TYPE)
WASM_SRCS := $(filter-out   \
	$(SRC_DIR)/controller-linux.c \
	$(SRC_DIR)/threads.c,   \
	$(wildcard $(SRC_DIR)/*c)) $(SRC_DIR)/wasm/*.c $(TEXTURE_C_FILES)

ifeq ($(PLATFORM), wasm)
	CFLAGS :=                                              \
		$(filter-out -ffast-math -march=native, $(CFLAGS)) \
		-MMD                                               \
		-MP                                                \
		-Isrc                                              \
		-I$(WASM_BUILD_DIR)                                \
		-D_BITS_STDIO_H                                    \
		-D__NO_CTYPE                                       \
		-D_BITS_PTHREADTYPES_COMMON_H                      \
		--target=wasm32                                    \
		--sysroot=/usr                                     \
		--no-standard-libraries                            \
		-Wl,--no-entry                                     \
		-Wl,--allow-undefined                              \
		-Wl,--export=wasm_main                             \
		-Wl,--export=run_callback                          \
		-Wl,--export=run_callback_int                      \
		-Wl,--export=run_callback_ptr                      \
		-Wl,--import-memory
	ifeq ($(BUILD_TYPE), release)
		CFLAGS += -Wl,--strip-all -Wl,-O3 -Wl,--lto-O3
	endif
endif

WASM_OPT_FLAGS := \
	--asyncify    \
	--pass-arg=asyncify-imports@env.JS_wait_for_next_frame,env.JS_usleep
ifeq ($(BUILD_TYPE), release)
	WASM_OPT_FLAGS += -O3
endif

TESTS_DIR := tests
TESTS_BUILD_DIR := $(BASE_BUILD_DIR)/tests
TESTS_EXEC := $(TESTS_BUILD_DIR)/test
TESTS_LIBS := check
TESTS_CFLAGS := -I$(SRC_DIR) $(shell pkg-config --cflags $(TESTS_LIBS))
TESTS_LDFLAGS := $(LDFLAGS) $(shell pkg-config --libs $(TESTS_LIBS))
TESTS_OBJS := $(patsubst $(TESTS_DIR)/%.c,$(TESTS_BUILD_DIR)/%.o,$(wildcard $(TESTS_DIR)/*.c))

.PHONY: all wasm test dev lint format clean

all: $(EXEC)

$(EXEC): $(OBJS) $(TEXTURE_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR):
	mkdir --parents --verbose $@

# We need to keep division by 0 to make infinity.
$(BUILD_DIR)/collision.o: CFLAGS := $(filter-out -ffast-math, $(CFLAGS))

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

-include $(OBJS:.o=.d)

$(TEXTURE_OBJS): CFLAGS := $(CFLAGS) -Werror -I$(SRC_DIR)

$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

$(BUILD_DIR)/%.c: $(ASSETS_DIR)/%.ppm scripts/embed_texture.py | $(BUILD_DIR)
	scripts/embed_texture.py $< $@

-include $(TEXTURE_OBJS:.o=.d)

wasm:
	make PLATFORM=wasm wasm/src/main.wasm

wasm/src/main.wasm: $(WASM_BUILD_DIR)/main.wasm
	cp --verbose $(WASM_BUILD_DIR)/main.wasm wasm/src/

$(WASM_BUILD_DIR)/main.wasm: $(WASM_SRCS) $(SRC_DIR)/*.h $(SRC_DIR)/wasm/*.h \
							 $(WASM_BUILD_DIR)/errno_message.h               \
							 $(WASM_BUILD_DIR)/ctype_lookup_table.h |        \
							 $(BUILD_DIR)
	$(WASM_CC) $(CFLAGS) -o $@ $(WASM_SRCS)
	wasm-opt $@ -o $@ $(WASM_OPT_FLAGS)

$(WASM_BUILD_DIR)/errno_message.h: scripts/errno_message.py | $(WASM_BUILD_DIR)
	scripts/errno_message.py > $@

$(WASM_BUILD_DIR)/ctype_lookup_table.h: scripts/ctype_lookup_table.py | $(WASM_BUILD_DIR)
	scripts/ctype_lookup_table.py > $@

test: $(TESTS_EXEC)
	$(TESTS_EXEC)

$(TESTS_EXEC): $(TESTS_OBJS) $(filter-out $(BUILD_DIR)/main.o, $(OBJS)) $(TEXTURE_OBJS) | $(TESTS_BUILD_DIR)
	$(CC) $(TESTS_CFLAGS) $^ -o $@ $(TESTS_LDFLAGS)

$(TESTS_BUILD_DIR):
	mkdir --parents --verbose $@

$(TESTS_BUILD_DIR)/%.o: $(TESTS_DIR)/%.c | $(TESTS_BUILD_DIR)
	$(CC) $(CFLAGS) $(TESTS_CFLAGS) -MMD -MP -c -o $@ $<

-include $(TESTS_OBJS:.o=.d)

dev:
	cd wasm && npm run dev

compile_commands.json: Makefile
	bear --output $@ -- make all $(TESTS_EXEC) --always-make

lint: compile_commands.json
	clang-tidy --config-file=.clang-tidy $(SRC_DIR)/*.c $(SRC_DIR)/*.h $(TESTS_DIR)/*.h $(TESTS_DIR)/*.c

format:
	clang-format -style=file -i $(SRC_DIR)/*.h $(SRC_DIR)/*.c $(SRC_DIR)/wasm/*.h $(SRC_DIR)/wasm/*.c $(TESTS_DIR)/*.h $(TESTS_DIR)/*.c --verbose

clean:
	rm --force --recursive --verbose $(BASE_BUILD_DIR) compile_commands.json
