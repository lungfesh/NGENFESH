CXX := g++
CXXFLAGS := -std=c++17 -Wall -Iinclude
LDFLAGS := -lglfw -ldl -lGL -lpthread -lm

SRC_DIR := src
BIN_DIR := bin
OBJ_DIR := obj

TARGET := $(BIN_DIR)/ngenfesh

SRCS := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%,$(SRCS:.cpp=.o))
OBJS := $(OBJS:.c=.o)

$(shell mkdir -p $(BIN_DIR) $(OBJ_DIR))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)

.PHONY: all clean
