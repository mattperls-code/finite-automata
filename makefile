
CXX := g++
AR := ar
CXXFLAGS := -std=c++23 -Ilib -Isrc -Wall -Wextra -Wno-sign-compare

SRC_DIR := src
LIB_DIR := lib
APP_DIR := app
TEST_DIR := tests
BUILD_DIR := build

IMPL_SOURCES := $(shell find $(SRC_DIR) $(LIB_DIR) -name '*.cpp')
IMPL_OBJECTS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(IMPL_SOURCES))

APP_MAIN := $(APP_DIR)/main.cpp
TEST_SOURCES := $(shell find $(TEST_DIR) -name '*.cpp')

APP_TARGET := main
TEST_TARGET := test
LIB_TARGET := library.a

all: $(APP_TARGET) $(LIB_TARGET)

# Compile implementation files into object files.
$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Archive object files into a static library.
$(LIB_TARGET): $(IMPL_OBJECTS)
	$(AR) rcs $@ $^

# Demo links against the static library.
$(APP_TARGET): $(APP_MAIN) $(LIB_TARGET)
	$(CXX) $(CXXFLAGS) -o $@ $(APP_MAIN) ./$(LIB_TARGET)

# Tests link against the static library.
$(TEST_TARGET): $(TEST_SOURCES) $(LIB_TARGET)
	$(CXX) $(CXXFLAGS) -I/opt/homebrew/include -o $@ $(TEST_SOURCES) ./$(LIB_TARGET) -L/opt/homebrew/lib -lcatch2

.PHONY: all clean

clean:
	rm -rf $(BUILD_DIR) $(APP_TARGET) $(TEST_TARGET) $(LIB_TARGET)
