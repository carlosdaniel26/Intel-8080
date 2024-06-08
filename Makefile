# Compilador e flags
CXX = gcc
CXXFLAGS = -Wall -Wextra -pedantic -std=c11 -Iinclude

# Diretórios
SRC_DIR = src
INC_DIR = include
OBJ_DIR = build

# Arquivos fonte e objeto
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

# Executável
EXEC = $(OBJ_DIR)/main

all: clean $(EXEC) run

# Regra para criar o executável
$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $(EXEC)

# Regra para compilar arquivos objeto
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpeza
clean:
	rm -rf $(OBJ_DIR)

# Executar
run: $(EXEC)
	@./$(EXEC)
