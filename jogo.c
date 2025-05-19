#include <stdio.h>
#include <stdlib.h>

#include "jogo.h"
#include "tabuleiro.h"
#include "mensagem.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#define NUM_TESOUROS  8
#define TAM_TABULEIRO 8
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
char** alocaMatriz(int tam_matriz)
{
   char** M;
   M = (char**) malloc(tam_matriz * sizeof(char*));
   for (int i = 0; i < tam_matriz; i++)
      M[i] = (char*) malloc(tam_matriz * sizeof(char));

   return M;
}

void liberaMatriz(char** M, int tam_matriz)
{
   if (M != NULL){
      for (int i = 0; i < tam_matriz; i++) free(M[i]);
      free(M);
   }
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/*                               Execução do                                 */
/*                             Jogo do Tesouro                               */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */ 
void inicia_jogo_tesouro(int soquete, Usuario usuario)
{
   /* Variáveis do Jogo */
   PosicaoJogador_t coordenadas_jogador;
   coordenadas_jogador.l = 7;
   coordenadas_jogador.c = 0;
   unsigned char jogo_em_andamento   = 1;
   unsigned char transmissao_arquivo = 0;
   
   mensagem_t msg = (mensagem_t) malloc(132 * sizeof(unsigned char));

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
         unsigned char tipo_msg;
         switch(movimento){
            case 'w':
               coordenadas_jogador.l--;
               tipo_msg = MOVE_CIMA;
               break;
            case 'a':
               coordenadas_jogador.c--;
               tipo_msg = MOVE_ESQUERDA;
               break;
            case 's':
               coordenadas_jogador.l++;
               tipo_msg = MOVE_BAIXO;
               break;
            case 'd':
               coordenadas_jogador.c++;
               tipo_msg = MOVE_DIREITA;
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

         /*           - = Transmissão de Mensagens = -            */
         /* (1) Cliente -> Servidor: Movimento do Jogador 
          * (2) Servidor -> Cliente: Tem Tesouro ou Não
          *    Se Não Tem Tesouro:
          *       Acaba a Transmissão e volta ao passo (1)
          *    Se Tem Tesouro:
          *       Servidor -> Cliente: Tamanho do Arquivo 
          *       Servidor -> Cliente: Dados do Arquivo
          *       Enquanto Não Varrer todo o Arquivo:
          *          Servidor -> Cliente: Parte do Arquivo 
          *       Servidor -> Cliente: Avisa que o Arquivo Acabou
          *       Acaba a Transmissão e volta ao passo (1)        */

         /* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
          *                ⚠ ABAIXO DAQUI ESTÁ EM CONSTRUÇÃO ⚠
          * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
         cria_mensagem(msg, 0, 0, tipo_msg, NULL);
         envia_mensagem(msg, soquete);

         while (recebe_mensagem(msg, soquete, 3000) == -1)
         {
            if (MSG_TIPO(msg) == ACK) break;
            cria_mensagem(msg, 0, 0, tipo_msg, NULL);
            envia_mensagem(msg, soquete);
         }
         /* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
          *                         ⚠ FIM DA OBRA ⚠
          * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

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

         /*           - = Transmissão de Mensagens = -            */
         /* (1) Cliente -> Servidor: Movimento do Jogador 
          * (2) Servidor -> Cliente: Tem Tesouro ou Não
          *    Se Não Tem Tesouro:
          *       Acaba a Transmissão e volta ao passo (1)
          *    Se Tem Tesouro:
          *       Servidor -> Cliente: Tamanho do Arquivo 
          *       Servidor -> Cliente: Dados do Arquivo
          *       Enquanto Não Varrer todo o Arquivo:
          *          Servidor -> Cliente: Parte do Arquivo 
          *       Servidor -> Cliente: Avisa que o Arquivo Acabou
          *       Acaba a Transmissão e volta ao passo (1)        */

         /* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
          *                ⚠ ABAIXO DAQUI ESTÁ EM CONSTRUÇÃO ⚠
          * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
         while (1){
            if (recebe_mensagem(msg, soquete, 3000) != -1 &&
                MSG_TIPO(msg) == MOVE_BAIXO || MSG_TIPO(msg) == MOVE_CIMA ||
                MSG_TIPO(msg) == MOVE_DIREITA || MSG_TIPO(msg) == MOVE_ESQUERDA)
               break;
         }
         
         switch(MSG_TIPO(msg)){
            case MOVE_CIMA:
               coordenadas_jogador.l--;
               break;
            case MOVE_BAIXO:
               coordenadas_jogador.l++;
               break;
            case MOVE_ESQUERDA:
               coordenadas_jogador.c--;
               break;
            case MOVE_DIREITA:
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
         
         /* Envia Informação do Tesouro */
         if (MatrizTabuleiro[coordenadas_jogador.l][coordenadas_jogador.c] == 'X'){
            cria_mensagem(msg, 0, 0, OK, 0);
            envia_mensagem(msg, soquete);
         }
         else {
            cria_mensagem(msg, 0, 0, OK, 0);
            envia_mensagem(msg, soquete);
         }
         /* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
          *                         ⚠ FIM DA OBRA ⚠
          * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

         sleep(1);
         system("clear");
      }

      /* Jogou Terminou! */
      imprime_tabuleiro(MatrizTabuleiro, TAM_TABULEIRO);
      printf("               Fim de Jogo.             \n");
   }

   /* Libera a Memória das Estruturas */
   free(msg);
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */