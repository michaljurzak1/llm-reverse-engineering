CC = gcc
CFLAGS = -Wall -Wextra -I./include
AR = ar
ARFLAGS = rcs

SRC_DIR = src
BUILD_DIR = build
LIB_NAME = libutils.a

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR)/$(LIB_NAME) $(BUILD_DIR)/main

$(BUILD_DIR)/$(LIB_NAME): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(AR) $(ARFLAGS) $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/main: $(BUILD_DIR)/main.o $(BUILD_DIR)/$(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L$(BUILD_DIR) -lutils

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean 