/* Bibliotecas Padrão */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Bibliotecas Locais */
#include "defines.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void imprime_tabuleiro(char** MatrizTabuleiro, int tam)
{
   printf("          [Planeta do Tesouro]          \n");
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

void inicia_tabuleiro(char** MatrizTabuleiro, int tam)
{
   for (int i = 0; i < tam; i++)
      for (int j = 0; j < tam; j++)
         MatrizTabuleiro[i][j] = ' ';
}

void gera_tesouros(char** MatrizTabuleiro, int tam, int num_tesouros)
{
   srand(time(NULL));
   
   int conta_tesouros = 0;
   int linha, coluna;
   int espaco;
   while (conta_tesouros != num_tesouros){
      espaco = rand() % (tam * tam);
      linha  = espaco / tam;
      coluna = espaco % tam;

      if (linha != tam - 1 && coluna != 0 && MatrizTabuleiro[linha][coluna] == ' '){
         conta_tesouros++;
         MatrizTabuleiro[linha][coluna] = 'X';
      }
   }
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */