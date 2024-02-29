CC = g++
CLAGS = -Wall -Wextra -Wpedantic -std=c++17 -MMD

TEST_EXE = test
BUILD_DIR = build

OBJECTS = $(wildcard *.cpp)
OBJECTS := $(addprefix $(BUILD_DIR)/, $(OBJECTS:.cpp=.o))

all: .mkbuild $(BUILD_DIR)/$(TEST_EXE)

$(BUILD_DIR)/$(TEST_EXE): $(OBJECTS)
	@$(CC) $+ -lgtest_main -lgmock_main -lgtest -lgmock -o $@ 

$(BUILD_DIR)/%.o: %.cpp *.h
	@$(CC) $(CFLAGS) -c $< -o $@

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all clean run .mkbuild

clean:
	@rm -rf $(BUILD_DIR)

.mkbuild:
	@mkdir -p $(BUILD_DIR)

run: all
	./$(BUILD_DIR)/$(TEST_EXE)



