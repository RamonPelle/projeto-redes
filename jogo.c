#include <stdio.h>

#include "jogo.h"
#include "tabuleiro.h"

#define NUM_TESOUROS  8
#define TAM_TABULEIRO 8

char** alocaMatriz(int tam_matriz)
{
   char** M;
   M = (char**) malloc(TAM_TABULEIRO * sizeof(char*));
   for (int i = 0; i < TAM_TABULEIRO; i++)
      M[i] = (char*) malloc(TAM_TABULEIRO * sizeof(char));

   return M;
}

void inicia_jogo_tesouro(int soquete, Usuario usuario)
{
   /* Variáveis do Jogo */
   PosicaoJogador_t coordenadas_jogador;
   coordenadas_jogador.l = 7;
   coordenadas_jogador.c = 0;
   unsigned char jogo_em_andamento = 1;

   char* caminho_absoluto_tesouros[NUM_TESOUROS];
   char** MatrizTabuleiro = alocaMatriz(TAM_TABULEIRO);
   inicia_tabuleiro(MatrizTabuleiro, TAM_TABULEIRO);

   /* <Código do Cliente>
    * (1) A matriz do Tabuleiro contém apenas as posições já visitadas e também
    * mantém a posição do Jogador.
    * (2) O Cliente deve RECEBER mensagens do tipo ARQUIVO (Texto, Imagem
    * e Vídeo), não enviá-las.
    * (3) ... */
   if (usuario == CLIENTE){
      MatrizTabuleiro[coordenadas_jogador.l][coordenadas_jogador.c] = 'P';
      
      char movimento;

      while (jogo_em_andamento){
         imprime_tabuleiro(MatrizTabuleiro, TAM_TABULEIRO);

         printf("Realize o seu movimento, astronauta: ");
         scanf("%c", &movimento);
         getchar();

         MatrizTabuleiro[coordenadas_jogador.l][coordenadas_jogador.c] = '*';
         switch(movimento){
            case 'w':
               coordenadas_jogador.l--;
               break;
            case 'a':
               coordenadas_jogador.c--;
               break;
            case 's':
               coordenadas_jogador.l++;
               break;
            case 'd':
               coordenadas_jogador.c++;
               break;
            default:
               break;
         }
         
         /* Corrige as Coordenadas Inválidas! */
         if (coordenadas_jogador.l < 0)
            coordenadas_jogador.l = 0;
         if (coordenadas_jogador.l == TAM_TABULEIRO)
            coordenadas_jogador.l = TAM_TABULEIRO - 1;
         if (coordenadas_jogador.c < 0)
            coordenadas_jogador.c = 0;
         if (coordenadas_jogador.c == TAM_TABULEIRO)
            coordenadas_jogador.c = TAM_TABULEIRO - 1;

         /* Atualiza a Posição do Jogador! */
         MatrizTabuleiro[coordenadas_jogador.l][coordenadas_jogador.c] = 'P';

         /* . . . */

         sleep(1);
         system("clear");
      }
   }

   /* <Código do Servidor>
    * (1) A matriz do Tabuleiro contém apenas as localizações dos tesouros.
    * (2) O Cliente deve ENVIAR mensagens do tipo ARQUIVO (Texto, Imagem
    * e Vídeo), não recebê-las.
    * (3) ... */
   else if (usuario == SERVIDOR){
      /* Gera os Tesouros no Tabuleiro */
      gera_tesouros(MatrizTabuleiro, TAM_TABULEIRO, NUM_TESOUROS);

      while (jogo_em_andamento){
         imprime_tabuleiro(MatrizTabuleiro, TAM_TABULEIRO);

         /* . . . */

         sleep(5);
         system("clear");
      }
   }

   /* Tipo de Usuário Não-Identificado... */
   else {
      return;
   }
}