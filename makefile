CXX := g++
CXXFLAGS := -std=c++23 -Ilib -Isrc -Wall -Wextra -Wno-sign-compare

SRC_DIR := src
LIB_DIR := lib
APP_DIR := app
TEST_DIR := tests

IMPL_SOURCES := $(shell find $(SRC_DIR) $(LIB_DIR) -name '*.cpp')

APP_MAIN := $(APP_DIR)/main.cpp

TEST_SOURCES := $(shell find $(TEST_DIR) -name '*.cpp')

APP_TARGET := main
TEST_TARGET := test

all: $(APP_TARGET)

$(APP_TARGET): $(APP_MAIN) $(IMPL_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST_TARGET): $(TEST_SOURCES) $(IMPL_SOURCES)
	$(CXX) $(CXXFLAGS) -I/opt/homebrew/include -o $@ $^ -L/opt/homebrew/lib -lcatch2

.PHONY: all clean
clean:
	rm -f $(APP_TARGET) $(TEST_TARGET)