# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -Iinclude
DEBUGFLAGS := -g
RELEASEFLAGS := -O2

# Directories
SRC_DIR := src
INCLUDE_DIR := include
BUILD_DIR := build
BIN_DIR := bin

# Executable name
TARGET := fractal-tree

# Automatically detect source and object files
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Default target: release build
.PHONY: all
all: release

# -----------------------------
# Release build
# -----------------------------
.PHONY: release
release: CFLAGS += $(RELEASEFLAGS)
release: dirs $(BIN_DIR)/$(TARGET)

# -----------------------------
# Debug build
# -----------------------------
.PHONY: debug
debug: CFLAGS += $(DEBUGFLAGS)
debug: dirs $(BIN_DIR)/$(TARGET)

# -----------------------------
# Build executable
# -----------------------------
$(BIN_DIR)/$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

# -----------------------------
# Build object files with dependency tracking
# -----------------------------
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Include dependency files
-include $(OBJS:.o=.d)

# -----------------------------
# Create necessary directories
# -----------------------------
.PHONY: dirs
dirs:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)

# -----------------------------
# Clean build artifacts
# -----------------------------
.PHONY: clean
clean:
	$(RM) -r $(BUILD_DIR)/* $(BIN_DIR)/*
