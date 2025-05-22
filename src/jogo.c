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

   /* Flags de Controle do Programa */
   int validade;
   unsigned char jogo_em_andamento   = 1;
   unsigned char transmissao_arquivo = 0;
   unsigned char acabou_rodada       = 0;
   unsigned char envia_ou_recebe     = 0;

   mensagem_t msg_enviar   = (mensagem_t) malloc(132 * sizeof(unsigned char));
   mensagem_t msg_anterior = (mensagem_t) malloc(132 * sizeof(unsigned char));
   mensagem_t msg_recebida = (mensagem_t) malloc(132 * sizeof(unsigned char));
   mensagem_t msg_esperada = (mensagem_t) malloc(132 * sizeof(unsigned char));

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
               tipo_msg = MV_CM;
               break;
            case 'a':
               coordenadas_jogador.c--;
               tipo_msg = MV_EQ;
               break;
            case 's':
               coordenadas_jogador.l++;
               tipo_msg = MV_BX;
               break;
            case 'd':
               coordenadas_jogador.c++;
               tipo_msg = MV_DI;
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

         /* Inicializa as Flags e Variáveis da rodada inicial */
         envia_ou_recebe = ESTADO_ENVIA;

         /* Enquanto não acabou a RODADA:
          *    ...
          *                                  */
         while (!acabou_rodada){
            /* (1) Cliente ENVIA uma Mensagem */
            if (envia_ou_recebe == ESTADO_ENVIA){
               /* ... */
            }

            /* (2) Cliente RECEBE uma Mensagem */
            else if (envia_ou_recebe == ESTADO_RECEBE){
               validade = recebe_mensagem(msg_recebida, soquete, 1000);
               if (validade == MSG_TIMEOUT){
                  /* ... */
               }
               else if (validade == INVL_CHECKSUM){
                  /* ... */
               }
               else if (validade == MSG_VALIDA)
                  envia_ou_recebe = ESTADO_ENVIA;
            }
         }
         /* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
          *                         ⚠ FIM DA OBRA ⚠
          * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
         
         /* Fim de jogo! Parabéns por completar o nosso jogo! */
         if (fim_de_jogo(MatrizTabuleiro, TAM_TABULEIRO))
            jogo_em_andamento = 0;

         sleep(3);
         system("clear");
      }
      
      /* Jogou Terminou! */
      imprime_tabuleiro(MatrizTabuleiro, TAM_TABULEIRO);
      printf("               Fim de Jogo.             \n");
   }

   /* <Código do Servidor>
    * (1) A matriz do Tabuleiro contém apenas as localizações dos tesouros.
    * (2) O Cliente deve ENVIAR mensagens do tipo ARQUIVO (Texto, Imagem
    * e Vídeo), não recebê-las.
    * (3) ... */
   else if (usuario == SERVIDOR){
      /* Variáveis específicas do Servidor */
      int inicio_da_rodada;

      /* Gera os Tesouros no Tabuleiro */
      gera_tesouros(MatrizTabuleiro, TAM_TABULEIRO, NUM_TESOUROS);

      while (jogo_em_andamento){
         /* Início da Rodada */
         inicio_da_rodada = 1;
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

         /* Inicializa as Flags e Variáveis da rodada inicial */
         envia_ou_recebe = ESTADO_RECEBE;

         /* Enquanto não acabou a RODADA:
          *    ...
          *                                  */
         while (!acabou_rodada){
            /* (1) Servidor ENVIA uma Mensagem */
            if (envia_ou_recebe == ESTADO_ENVIA){
               /* . . . */
            }

            /* (2) Servidor RECEBE uma Mensagem */
            else if (envia_ou_recebe == ESTADO_RECEBE){
               validade = recebe_mensagem(msg_recebida, soquete, 1000);
               if (validade == MSG_TIMEOUT){
                  /* ... */
               }
               else if (validade == INVL_CHECKSUM){
                  /* ... */
               }
               else if (validade == MSG_VALIDA)
                  envia_ou_recebe = ESTADO_ENVIA;
            }
         }

         /* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
          *                         ⚠ FIM DA OBRA ⚠
          * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

         /* Fim de jogo! Parabéns por completar o nosso jogo! */
         if (fim_de_jogo(MatrizTabuleiro, TAM_TABULEIRO))
            jogo_em_andamento = 0;
         
         sleep(1);
         system("clear");
      }

      /* Jogou Terminou! */
      imprime_tabuleiro(MatrizTabuleiro, TAM_TABULEIRO);
      printf("               Fim de Jogo.             \n");
   }

   /* Libera a Memória das Estruturas */
   liberaMatriz(MatrizTabuleiro, TAM_TABULEIRO);
   free(msg_enviar);
   free(msg_anterior);
   free(msg_recebida);
   free(msg_esperada);
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */