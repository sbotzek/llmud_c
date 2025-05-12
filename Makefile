CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -Iinclude \
         -g -O0 -fno-inline -fno-omit-frame-pointer
LDLIBS = -lcrypt

SRC_DIR = src
OBJ_DIR = obj
TARGET = mud

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Test configuration
TEST_DIR = test
TEST_BUILD_DIR = $(OBJ_DIR)/test
TEST_SRC = $(filter-out $(SRC_DIR)/main.c, $(SRCS))
TESTS = $(wildcard $(TEST_DIR)/test_*.c)
TEST_BINS = $(patsubst $(TEST_DIR)/%.c,$(TEST_BUILD_DIR)/%,$(TESTS))

# Default target
all: $(TARGET)

# Build the game binary
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

# Compile .o files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure build directories exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(TEST_BUILD_DIR):
	mkdir -p $(TEST_BUILD_DIR)

# Build test binaries (all src/*.c except main.c)
$(TEST_BUILD_DIR)/test_%: $(TEST_DIR)/test_%.c $(TEST_SRC) | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

# Run all tests
test: $(TEST_BINS)
	@echo "Running tests..."
	@for t in $(TEST_BINS); do echo $$t; ./$$t || exit 1; done
	@echo "All tests passed."

# Clean all build artifacts
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

integration-test: $(TARGET)
	cd integration && { \
		../$(TARGET) --test-mode > mud_server.log 2>&1 & \
		sleep 1; \
		python3 test_runner.py tests/**/*.json; \
		kill %1; \
	}
