CXX := g++
CXXFLAGS := -std=c++17 -O3 -I./base/include
LDFLAGS := -static -static-libgcc -static-libstdc++
SRC_DIR :=  ./base/src
BUILD_DIR := ./build
TARGET := $(BUILD_DIR)/2048

SRCS := $(foreach dir,$(SRC_DIR),$(wildcard $(dir)/*.cpp))
OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))

all: clean_objs $(TARGET)
	@echo "Build completed successfully."
	@$(MAKE) --no-print-directory clean_objs

$(TARGET): $(OBJS) | create_build_dir
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cpp | create_build_dir
	$(CXX) $(CXXFLAGS) -c $< -o $@

create_build_dir:
	mkdir -p $(dir $(OBJS))

clean_objs:
	rm -rf $(BUILD_DIR)/base

	