#ifndef __TABULEIRO_H_
#define __TABULEIRO_H_

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
typedef struct {
   FILE* arq_tesouro;
   char* nome_tesouro;
} Tesouro_t;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Funções de Manipulação do Tabuleiro */
void imprime_tabuleiro (char** tesouros_tabuleiro, int tam_tab);
void inicia_tabuleiro  (char** MatrizTabuleiro, int tam);
void gera_tesouros     (char** tesouros_tabuleiro, int tam_tab, int num_tesouros);
void abre_tesouros     (Tesouro_t* Tesouros, int num_tesouros);
int  fim_de_jogo       (char** tesouros_tabuleiro, int tam_tab);
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#endif