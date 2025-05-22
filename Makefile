# /* - = - = - = Disciplina CI1058: Redes I = - = - = - */
# // Docentes: Albini & Todt
# // Discente 01: Leonardo Amorim Carpwiski (GRR 20232331)
# // Discente 02: Ramon Pelle               (GRR 20232379)
# /* - = - = - = - = - = - = - = - = - = - = - = - = - */

# Curitiba, Paraná - Maio/Junho, 2025

# -=-=-=-=-=-=-=-=-=-=-
# Variáveis do Arquivo
# -=-=-=-=-=-=-=-=-=-=-
# Nome do Projeto
PROJ_NAME=jogo_tesouro

# Diretórios
SRC_DIR = src
BUILD_DIR = build

# Arquivos .c
C_SOURCE=$(wildcard *.c)

# Arquivos .h
H_SOURCE=$(wildcard *.h)

# Bibliotecas
LIB_SOURCE =

# Objetos
OBJ=$(C_SOURCE:.c=.o)

# Compilador
CC=gcc

# Flags do Compilador
CC_FLAGS=-c -Wall -O2 -g


# -=-=-=-=-=-=-=-=-=-=-
# Comandos de Execução
# -=-=-=-=-=-=-=-=-=-=-
# all: Compila o projeto
all: $(PROJ_NAME)

$(PROJ_NAME): $(OBJ)
	$(CC) -o $@ $^ $(LIB_SOURCE)

%.o: %.c
	$(CC) $(CC_FLAGS) -o $@ $^

# clean: Remove os arquivos objeto e temporários
clean:
	rm -rf *.o $(PROJ_NAME) *~
