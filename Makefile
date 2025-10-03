CXX := g++
PKG_CONFIG := pkg-config

SRC_DIR := src
INC_DIR := include
BUILD_DIR := build
TARGET := $(BUILD_DIR)/spoofeye

SOURCES := $(shell find $(SRC_DIR) -name '*.cpp')
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
DEPS := $(OBJECTS:.o=.d)

# Compile flags
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -I$(INC_DIR) $(shell $(PKG_CONFIG) --cflags libnotify)
LDFLAGS := $(shell $(PKG_CONFIG) --libs libnotify) -lpcap

.PHONY: all clean

all: $(TARGET)

# Compile each .cpp to .o and generate dependencies
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Include dependency files
-include $(DEPS)

# Link executable
$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)
