# Makefile for building and running the NCO test

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2
# Link the math library
LDFLAGS = -lm

# Directories
SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests

# Files
TARGET = nco_test
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC_FILES))

# Rules
all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

test: $(BUILD_DIR)/$(TARGET)
	@echo "Running tests..."
	@./$(BUILD_DIR)/$(TARGET)

clean:
	@echo "Cleaning up..."
	@rm -rf $(BUILD_DIR)

.PHONY: all test clean

