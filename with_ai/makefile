CXX := g++
CXXFLAGS := -std=c++17 -O3 -fopenmp -I./ai/matrix/include -I./ai/neatGA/include -I./ai/simpleGA/include -I./ai/singlePoleBalancing -I./base/include -I./glfw/include -I./glHelper -I./imgui
LDFLAGS := -L./glfw/lib-mingw-w64 -l:libglfw3dll.a -lopengl32 -static -static-libgcc -static-libstdc++
SRC_DIR := ./ai/matrix/src ./ai/neatGA/src ./ai/simpleGA/src ./ai/singlePoleBalancing ./base/src ./glHelper ./imgui
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
	rm -rf $(BUILD_DIR)/ai $(BUILD_DIR)/base $(BUILD_DIR)/glHelper $(BUILD_DIR)/imgui

	