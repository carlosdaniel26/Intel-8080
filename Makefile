# Compiler and Flags
CXX = gcc
CXXFLAGS = -Wall -Wextra -pedantic -std=c11 -Iinclude `sdl2-config --cflags`
LDFLAGS = `sdl2-config --libs`

# DIR
SRC_DIR = src
INC_DIR = include
OBJ_DIR = build

# Sources and Objects
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

# Exec
EXEC = $(OBJ_DIR)/main

all: clean $(EXEC) run

# Exec to OBJ
$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $(EXEC) $(LDFLAGS)

# Compile OBJ
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

run: $(EXEC)
	@./$(EXEC)

