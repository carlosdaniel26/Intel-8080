CC = gcc
CCFLAGS = -Wall -Wextra -pedantic -std=c11 -Iinclude -O1
LDFLAGS = -lSDL2
DEBUG_FLAGS = -g

SRC_DIR = src
INC_DIR = include
OBJ_DIR = build

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

EXEC = $(OBJ_DIR)/main

$(EXEC): $(OBJ)
	@echo "(LD) $@"
	@$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS) $(DEBUG_FLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	@echo "(CC) $<"
	@$(CC) $(CCFLAGS) $(DEBUG_FLAGS) -c $< -o $@

all: $(EXEC)

debug: $(EXEC)
	@gdb -x gdb_script.gdb --tui ./$(EXEC)

dev: $(EXEC) run

test: $(EXEC)
	$(MAKE)
	@./$(EXEC)

clean:
	@rm -rf $(OBJ_DIR)

run: $(EXEC)
	@./$(EXEC)