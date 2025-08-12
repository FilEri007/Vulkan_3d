CXX = g++
CXXFLAGS_DEBUG = -std=c++17 -Wall -Wextra -Iinclude -g
CXXFLAGS_RELEASE = -std=c++17 -Wall -Wextra -Iinclude -O2 -DNDEBUG
LDFLAGS = -Llib -lvulkan-1 -lSDL3

SRC = $(wildcard src/*.cpp)
BIN_DIR = bin
OUT = $(BIN_DIR)/main.exe

debug: CXXFLAGS=$(CXXFLAGS_DEBUG)
debug: $(OUT)

release: CXXFLAGS=$(CXXFLAGS_RELEASE)
release: $(OUT)

$(OUT): $(SRC) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -f $(OUT)