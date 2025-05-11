/* Bibliotecas Padrão */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Bibliotecas Locais */
#include "defines.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void imprime_tabuleiro(char** tesouros_tabuleiro)
{
   printf("   ┌───┬───┬───┬───┬───┬───┬───┬───┐ \n");

   for (int i = 0; i < 8; i++) {
      printf(" %d ", 8 - i - 1);
      for (int j = 0; j < 8; j++)
         printf("│ %c ", tesouros_tabuleiro[i][j]);
      printf("│\n");
      if (i != 7) printf("   ├───┼───┼───┼───┼───┼───┼───┼───┤ \n");
   }
   
   printf("   └───┴───┴───┴───┴───┴───┴───┴───┘ \n"
          "     0   1   2   3   4   5   6   7   \n");
}

void gera_tesouros(char** tesouros_tabuleiro, int num_tesouros)
{
   for (int i = 0; i < 8; i++)
      for (int j = 0; j < 8; j++)
         tesouros_tabuleiro[i][j] = ' ';
   
   int conta_tesouros = 0;
   int linha, coluna;
   int espaco;
   while (conta_tesouros != num_tesouros){
      espaco = rand() % 64;
      linha  = espaco / 8;
      coluna = espaco % 8;
      if (tesouros_tabuleiro[linha][coluna] == ' '){
         conta_tesouros++;
         tesouros_tabuleiro[linha][coluna] = 'X';
      }
   }
}