CXX      := g++
CXXFLAGS := -Iinclude
LDFLAGS  := -lglfw -lGL -ldl

SRC := src/main.cpp src/glad.c
OUT := main

all: $(OUT)

$(OUT): $(SRC)
	$(CXX) $(SRC) -o $(OUT) $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f $(OUT)

.PHONY: all clean