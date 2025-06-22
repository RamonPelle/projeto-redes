#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "tabuleiro.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/*                               Controle do                                 */
/*                                Tabuleiro                                  */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* [Função] imprime_tabuleiro()
 * Descrição: */
void imprime_tabuleiro(char** MatrizTabuleiro, int tam)
{
   printf("\n          [Planeta do Tesouro]          \n");

   printf("   ┌───┬───┬───┬───┬───┬───┬───┬───┐ \n");

   for (int i = 0; i < tam; i++) {
      printf(" %d ", tam - i - 1);
      for (int j = 0; j < tam; j++)
         printf("│ %c ", MatrizTabuleiro[i][j]);
      printf("│\n");
      if (i != tam - 1) printf("   ├───┼───┼───┼───┼───┼───┼───┼───┤ \n");
   }
   
   printf("   └───┴───┴───┴───┴───┴───┴───┴───┘ \n"
          "     0   1   2   3   4   5   6   7   \n");
}

/* [Função] inicia_tabuleiro()
 * Descrição: */
void inicia_tabuleiro(char** MatrizTabuleiro, int tam)
{
   for (int i = 0; i < tam; i++)
      for (int j = 0; j < tam; j++)
         MatrizTabuleiro[i][j] = ' ';
}

/* [Função] gera_tesouros()
 * Descrição: */
void gera_tesouros(char** MatrizTabuleiro, int tam, int num_tesouros)
{
   srand(time(NULL));
   
   MatrizTabuleiro[tam - 1][0] = 'P';
   
   int conta_tesouros = 0;
   int linha, coluna;
   int espaco;
   while (conta_tesouros != num_tesouros){
      espaco = rand() % (tam * tam);
      linha  = espaco / tam;
      coluna = espaco % tam;

      if (!(linha == tam - 1 && coluna == 0) && MatrizTabuleiro[linha][coluna] == ' '){
         MatrizTabuleiro[linha][coluna] = 49 + conta_tesouros;
         conta_tesouros++;
      }
   }
}

/* [Função] abre_tesouros()
 * Descrição: Guarda os descritores de arquivos em um vetor de tesouros. Se o
 programa não conseguiu abrir o arquivo do tesouro, guarda NULL. */
void abre_tesouros(Tesouro_t* Tesouros, int num_tesouros){
   for (int i = 0; i < num_tesouros; i++){
      Tesouros[i].arq_tesouro = NULL;
      Tesouros[i].nome_tesouro = (char*) malloc(6 * sizeof(char));
      strcpy(Tesouros[i].nome_tesouro, "K.ana");
   }
   
   char* local_tesouro =   (char*) malloc(16 * sizeof(char));
   char* abrindo_tesouro = (char*) malloc(25 * sizeof(char));
   strcpy(local_tesouro, "./objetos/N.ext");

   FILE* f;
   for (int i = 0; i < num_tesouros; i++){
      local_tesouro[10]   = 49 + i;
      abrindo_tesouro[19] = 49 + i;

      strcpy(&local_tesouro[12], "jpg");
      f = fopen(local_tesouro, "rb");
      if (f){
         Tesouros[i].arq_tesouro = f;
         strcpy(Tesouros[i].nome_tesouro, &local_tesouro[10]);
      }

      strcpy(&local_tesouro[12], "mp4");
      f = fopen(local_tesouro, "rb");
      if (f){
         Tesouros[i].arq_tesouro = f;
         strcpy(Tesouros[i].nome_tesouro, &local_tesouro[10]);
      }

      strcpy(&local_tesouro[12], "txt");
      f = fopen(local_tesouro, "rb");
      if (f){
         Tesouros[i].arq_tesouro = f;
         strcpy(Tesouros[i].nome_tesouro, &local_tesouro[10]);
      }
   }
}

/* [Função] fim_de_jogo()
 * Descrição: Verifica se o tabuleiro inteiro já foi percorrido, analisando os
 elementos da matriz do tabuleiro. Se não há tesouros ou espaços em branco, en-
 tão todos os espaços já foram visitados, e o jogo deve ser encerrado. */
int fim_de_jogo(char** MatrizTabuleiro, int tam)
{
   int espacos_nao_visitados = 0;

   for (int i = 0; i < tam; i++)
      for (int j = 0; j < tam; j++)
         if (MatrizTabuleiro[i][j] == ' ') espacos_nao_visitados++;

   if     (espacos_nao_visitados == 0) return 1;
   else if (espacos_nao_visitados > 0) return 0;

   return 0;
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */