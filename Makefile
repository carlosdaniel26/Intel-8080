CXX = gcc
CXXFLAGS = -Wall -Werror -Wextra -pedantic -std=c11 -Iinclude
LDFLAGS = -lSDL2
DEBUG_FLAGS = -g

SRC_DIR = src
OBJ_DIR = build

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

EXEC = $(OBJ_DIR)/main

all: $(EXEC)

debug: clean $(EXEC)
	gdb -tui -x gdb_script.gdb ./$(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $(EXEC) $(LDFLAGS) $(DEBUG_FLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

run: $(EXEC)
	@./$(EXEC)
