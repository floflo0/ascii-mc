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
else
$(error Invalid PLATFORM. Expected 'linux-x86_64', but got: $(PLATFORM))
endif

CC := gcc
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
BUILD_DIR = $(BASE_BUILD_DIR)/$(PLATFORM)/$(BUILD_TYPE)
ASSETS_DIR := assets
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(wildcard $(SRC_DIR)/*.c))
TEXTURE_OBJS := $(patsubst $(ASSETS_DIR)/%.ppm,$(BUILD_DIR)/%.o,$(wildcard $(ASSETS_DIR)/*.ppm))
EXEC := $(BUILD_DIR)/$(NAME)

TESTS_DIR := tests
TESTS_BUILD_DIR := $(BASE_BUILD_DIR)/tests
TESTS_EXEC := $(TESTS_BUILD_DIR)/test
TESTS_LIBS := check
TESTS_CFLAGS := -I$(SRC_DIR) $(shell pkg-config --cflags $(TESTS_LIBS))
TESTS_LDFLAGS := $(LDFLAGS) $(shell pkg-config --libs $(TESTS_LIBS))
TESTS_OBJS := $(patsubst $(TESTS_DIR)/%.c,$(TESTS_BUILD_DIR)/%.o,$(wildcard $(TESTS_DIR)/*.c))

.PHONY: all test lint format clean

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

test: $(TESTS_EXEC)
	$(TESTS_EXEC)

$(TESTS_EXEC): $(TESTS_OBJS) $(filter-out $(BUILD_DIR)/main.o, $(OBJS)) $(TEXTURE_OBJS) | $(TESTS_BUILD_DIR)
	$(CC) $(TESTS_CFLAGS) $^ -o $@ $(TESTS_LDFLAGS)

$(TESTS_BUILD_DIR):
	mkdir --parents --verbose $@

$(TESTS_BUILD_DIR)/%.o: $(TESTS_DIR)/%.c | $(TESTS_BUILD_DIR)
	$(CC) $(CFLAGS) $(TESTS_CFLAGS) -MMD -MP -c -o $@ $<

-include $(TESTS_OBJS:.o=.d)

compile_commands.json: Makefile
	bear --output $@ -- make all $(TESTS_EXEC) --always-make

lint: compile_commands.json
	clang-tidy --config-file=.clang-tidy $(SRC_DIR)/*.c $(SRC_DIR)/*.h $(TESTS_DIR)/*.h $(TESTS_DIR)/*.c

format:
	clang-format -style=file -i $(SRC_DIR)/*.h $(SRC_DIR)/*.c $(TESTS_DIR)/*.h $(TESTS_DIR)/*.c --verbose

clean:
	rm --force --recursive --verbose $(BASE_BUILD_DIR) compile_commands.json
