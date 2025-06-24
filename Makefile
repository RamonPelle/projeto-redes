# /* - = - = - = Disciplina CI1058: Redes I = - = - = - */
# // Docentes: Albini & Todt
# // Discente 01: Leonardo Amorim Carpwiski (GRR 20232331)
# // Discente 02: Ramon Pelle               (GRR 20244408)
# /* - = - = - = - = - = - = - = - = - = - = - = - = - */

# Curitiba, Paraná - Maio/Junho, 2025

# -=-=-=-=-=-=-=-=-=-=-
# Variáveis do Arquivo
# -=-=-=-=-=-=-=-=-=-=-
# Define o compilador C
CC = gcc

# Define os diretórios
SRC_DIR = src
BUILD_DIR = build

# Define o nome do executável
TARGET = jogo_tesouro

# Encontra todos os arquivos .c no diretório SRC_DIR
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Cria os caminhos para os arquivos .o no diretório BUILD_DIR
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Flags de compilação
# -I$(SRC_DIR) adiciona o diretório src para procurar includes (.h)
# -Wall habilita todos os avisos
# -g adiciona informações de depuração
CFLAGS = -I$(SRC_DIR) -Wall -g

# Regra padrão: compila e linka o executável
all: $(TARGET)

# Regra para criar o executável
$(TARGET): $(OBJS)
	@echo "Ligando arquivos..."
	$(CC) $(OBJS) -o $(TARGET)
	@echo "Executável '$(TARGET)' criado com sucesso!"

# Regra para compilar arquivos .c em .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR) # Garante que o diretório build exista
	@echo "Compilando $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para limpar os arquivos gerados
clean:
	@echo "Limpando a sujeira..."
	@rm -f $(TARGET)
	@rm -rf $(BUILD_DIR)
	@echo "Limpeza completa."

.PHONY: all clean
