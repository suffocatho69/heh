# =========================================================================
#  RETRO 2D NESTING SYSTEM MAKEFILE
#  Supports both native Linux compilation (CLI Test) and MinGW
#  cross-compilation for Windows (both CLI and Win32 GUI).
# =========================================================================

CXX = g++
CXXFLAGS = -std=c++14 -O2 -Wall -Wextra -pthread
LIBS = -lpthread

# Target directories
SRC_DIR = src
OBJ_DIR = obj

# Source files
SRCS = $(SRC_DIR)/Component.cpp \
       $(SRC_DIR)/DXFReader.cpp \
       $(SRC_DIR)/NestingEngine.cpp \
       $(SRC_DIR)/ComponentManager.cpp \
       $(SRC_DIR)/NCGenerator.cpp

# Output binary names
TEST_BIN = test_suite
GUI_BIN = Nesting2D.exe

# Cross compilation setup (if requested)
WIN_CXX = i686-w64-mingw32-g++
WIN_RC  = i686-w64-mingw32-windres
WIN_LIBS = -mwindows -lcomctl32 -lgdi32 -lpthread

.PHONY: all test clean win32

# Default target compiles the Linux verification test suite
all: test

test: $(TEST_BIN)
	@echo "========================================================="
	@echo "  Running Verification Tests"
	@echo "========================================================="
	./$(TEST_BIN)

$(TEST_BIN): test_main.cpp $(SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

# Compile Windows binary using cross-compiler
win32: $(GUI_BIN)

$(GUI_BIN): $(SRCS) $(SRC_DIR)/MainWindow.cpp $(SRC_DIR)/main.cpp $(SRC_DIR)/resource.rc
	$(WIN_RC) $(SRC_DIR)/resource.rc -O coff -o $(SRC_DIR)/resource.res
	$(WIN_CXX) $(CXXFLAGS) $^ $(SRC_DIR)/resource.res -o $@ $(WIN_LIBS)

clean:
	rm -f $(TEST_BIN) $(GUI_BIN) $(SRC_DIR)/*.res $(SRC_DIR)/*.o
