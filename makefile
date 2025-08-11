CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
LDFLAGS = -Llib -lvulkan-1

SRC = $(wildcard src/*.cpp)
BIN_DIR = bin
OUT = $(BIN_DIR)/main.exe

$(OUT): $(SRC) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -f $(OUT)