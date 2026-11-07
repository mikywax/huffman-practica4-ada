.PHONY: all clean test

PROJECT_DIR := huffman-practica4-ada
CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
TARGET := $(PROJECT_DIR)/huffman
SRC := $(PROJECT_DIR)/src/huffman_tree.cpp $(PROJECT_DIR)/src/codec.cpp $(PROJECT_DIR)/src/main.cpp
OBJ := $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(PROJECT_DIR)/%.o: $(PROJECT_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(PROJECT_DIR)/include -c -o $@ $<

clean:
	rm -f $(OBJ) $(TARGET)

test: all
	bash $(PROJECT_DIR)/test/run_tests.sh