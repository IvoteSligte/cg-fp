CXX = g++
CXXFLAGS = -g -Wall -Wextra -std=c++17 $(shell pkg-config --cflags sdl2)
LDFLAGS = -lGL -lGLEW $(shell pkg-config --libs sdl2)

SRC = main.cpp
BIN = main

all: main

$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -rf $(BIN)

