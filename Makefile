BUILD_TYPE ?= debug

NAME := ascii-mc
VERSION := 0.0.0

CC := gcc
LIBS := libevdev libsystemd
CFLAGS +=                                \
	-Wall                                \
	-Wextra                              \
	-Wcast-qual                          \
	-Wmissing-prototypes                 \
	-DVERSION=\"$(VERSION)\"             \
	$(shell pkg-config --cflags $(LIBS)) \
	$(EXTRA_CFLAGS)
ifeq ($(BUILD_TYPE), release)
	CFLAGS += -DPROD -DNDEBUG -Ofast -flto=auto -march=native
else ifeq ($(BUILD_TYPE), debug)
	CFLAGS += -ggdb
else
	$(error Invalid BUILD_TYPE value. Expected 'debug' or 'release', but got: $(BUILD_TYPE))
endif
LDFLAGS := $(shell pkg-config --libs $(LIBS)) -lm -lpthread
SRC_DIR := src
BUILD_DIR := build
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(wildcard $(SRC_DIR)/*.c))
EXEC := $(BUILD_DIR)/$(NAME)

TESTS_DIR := tests
TESTS_EXEC := $(TESTS_DIR)/test
TEST_LIBS := check
TEST_CFLAGS := $(CFLAGS) -I$(SRC_DIR) $(shell pkg-config --cflags $(TEST_LIBS))
TEST_LDFLAGS := $(LDFLAGS) $(shell pkg-config --libs $(TEST_LIBS))
TESTS_OBJS := $(patsubst %.c,%.o,$(wildcard $(TESTS_DIR)/*.c))

.PHONY: all test lint format clean

all: $(EXEC)

$(BUILD_DIR):
	mkdir --parents --verbose $(BUILD_DIR)

$(EXEC): $(OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

-include $(OBJS:.o=.d)

# We need to keep division by 0 to make infinity.
$(BUILD_DIR)/collision.o: CFLAGS := $(CFLAGS) -fno-fast-math

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

-include $(TESTS_OBJS:.o=.d)

$(TESTS_DIR)/%.o: CFLAGS := $(TEST_CFLAGS)

$(TESTS_EXEC): $(TESTS_OBJS) $(filter-out $(BUILD_DIR)/main.o, $(OBJS)) $(TEXTURE_OBJS)
	$(CC) $(TEST_CFLAGS) $^ -o $@ $(TEST_LDFLAGS)

test: $(TESTS_EXEC)
	$(TESTS_EXEC)

compile_commands.json: Makefile
	bear --output $@ -- make all $(TESTS_EXEC) --always-make

lint: compile_commands.json
	clang-tidy --config-file=.clang-tidy $(SRC_DIR)/*.c $(SRC_DIR)/*.h $(TESTS_DIR)/*.h $(TESTS_DIR)/*.c

format:
	clang-format -style=file -i $(SRC_DIR)/*.h $(SRC_DIR)/*.c $(TESTS_DIR)/*.h $(TESTS_DIR)/*.c --verbose

clean:
	rm --force --recursive --verbose $(BUILD_DIR) $(TESTS_EXEC) $(TESTS_DIR)/*.o $(TESTS_DIR)/*.d compile_commands.json
