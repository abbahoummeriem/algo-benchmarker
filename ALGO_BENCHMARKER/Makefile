CC ?= gcc
SRC_DIR = src
INC_DIR = include
CFLAGS ?= -std=c11 -O3 -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -I$(INC_DIR) -MMD -MP
LDFLAGS ?= -lm
OBJ_DIR = obj
BIN_DIR = bin
PLOTS_DIR = plots

SRCS = $(SRC_DIR)/algorithms.c \
       $(SRC_DIR)/benchmark.c \
       $(SRC_DIR)/main.c

OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
DEPS = $(OBJS:.o=.d)
TARGET = $(BIN_DIR)/algo_benchmarker

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "\033[1;32m[BUILD SUCCESS]\033[0m Binary created at $@"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

$(BIN_DIR) $(OBJ_DIR) $(PLOTS_DIR):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	rm -f $(PLOTS_DIR)/*.png $(PLOTS_DIR)/*.gp
	@echo "Cleaned build artifacts."

test: $(TARGET)
	@echo "Testing Mini Algo Benchmarker..."
	./$(TARGET)
