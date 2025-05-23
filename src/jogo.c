#include <stdio.h>
#include <stdlib.h>

#include "jogo.h"
#include "tabuleiro.h"
#include "mensagem.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#define NUM_TESOUROS  8
#define TAM_TABULEIRO 8
#define TEMPO_TIMEOUT 500
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Funções Auxiliares da execução do jogo */
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
   PosicaoJogador_t coord_jogador;
   coord_jogador.l = 7;
   coord_jogador.c = 0;

   /* Flags de Controle do Programa */
   int validade;
   unsigned char inicio_da_rodada;
   unsigned char jogo_em_andamento   = 1;
   unsigned char transmissao_arquivo = 0;
   unsigned char acabou_rodada       = 0;
   unsigned char envia_ou_recebe     = 0;
   unsigned char reenvia_mensagem;
   unsigned char mensagem_com_erro;

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
      MatrizTabuleiro[coord_jogador.l][coord_jogador.c] = 'P';
      
      char movimento;
      while (jogo_em_andamento){
         imprime_tabuleiro(MatrizTabuleiro, TAM_TABULEIRO);

         printf("Realize o seu movimento, astronauta: ");
         scanf("%c", &movimento);
         getchar();

         MatrizTabuleiro[coord_jogador.l][coord_jogador.c] = '*';
         unsigned char tipo_msg;
         switch(movimento){
            case 'w':
               coord_jogador.l--;
               tipo_msg = MV_CM;
               break;
            case 'a':
               coord_jogador.c--;
               tipo_msg = MV_EQ;
               break;
            case 's':
               coord_jogador.l++;
               tipo_msg = MV_BX;
               break;
            case 'd':
               coord_jogador.c++;
               tipo_msg = MV_DI;
               break;
            default:
               break;
         }
         
         /* Corrige as Coordenadas Inválidas! */
         if (coord_jogador.l < 0)
            coord_jogador.l = 0;
         if (coord_jogador.l == TAM_TABULEIRO)
            coord_jogador.l = TAM_TABULEIRO - 1;
         if (coord_jogador.c < 0)
            coord_jogador.c = 0;
         if (coord_jogador.c == TAM_TABULEIRO)
            coord_jogador.c = TAM_TABULEIRO - 1;

         /* Atualiza a Posição do Jogador! */
         MatrizTabuleiro[coord_jogador.l][coord_jogador.c] = 'P';

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

         /* Inicializa as Flags e Variáveis da rodada */
         envia_ou_recebe     = ESTADO_ENVIA;
         inicio_da_rodada    = 1;
         transmissao_arquivo = 0;
         acabou_rodada       = 0;
         reenvia_mensagem    = 0;
         mensagem_com_erro   = 0;

         while (!acabou_rodada){
            /* (1) Cliente ENVIA uma Mensagem 
             * 〖 Listagem das Possibilidades 〗
             * Nome da Mensagem | Descrição
             * -------------------------------------------------------------
             * MV               | Movimento (Esquerda, Direita, Cima, Baixo)
             * ACK              | Acknownlegde
             * NACK             | Not Acknownlegde
             * FIM_RODADA       | Fim da Rodada atual
             * ERRO             | Erro (Espaço Insuficiente)
             * 
             * 〖 Tabela de Envio de Mensagens 〗
             * Mensagem RECEBIDA | Mensagem A ENVIAR 
             * -------------------------------------
             *                                                               */
            if (envia_ou_recebe == ESTADO_ENVIA){
               if (reenvia_mensagem == 1){
                  envia_mensagem(msg_anterior, soquete);
                  envia_ou_recebe = ESTADO_RECEBE;
                  reenvia_mensagem = 0;
               }
               else if (mensagem_com_erro == 1){
                  cria_mensagem(msg_enviar, 0, 0, NACK, NULL);
                  envia_mensagem(msg_enviar, soquete);
                  envia_ou_recebe = ESTADO_RECEBE;
                  mensagem_com_erro = 0;
               }
               else {
                  if (MSG_TIPO(msg_recebida) == TESOURO){
                     if (msg_recebida[5] == 0){
                        printf("\nTESOURO NAO FOI ENCONTRADO...\n");
                     }
                     else {
                        printf("\nTESOURO ENCONTRADO!!!\n");
                     }
                     cria_mensagem(msg_enviar, 0, 0, ACK, NULL);
                     envia_mensagem(msg_enviar, soquete);
                     envia_ou_recebe = ESTADO_RECEBE;
                  }
                  else if (MSG_TIPO(msg_recebida) == NACK){
                     reenvia_mensagem = 1;
                  }
               }
            }

            /* (2) Cliente RECEBE uma Mensagem */
            else if (envia_ou_recebe == ESTADO_RECEBE){
               validade = recebe_mensagem(msg_recebida, soquete, TEMPO_TIMEOUT);
               if (validade == MSG_TIMEOUT){
                  reenvia_mensagem = 1;
                  envia_ou_recebe  = ESTADO_ENVIA;
               }
               else if (validade == MSG_ERRO_CHECK){
                  mensagem_com_erro = 1;
                  envia_ou_recebe = ESTADO_ENVIA;
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

         /* Inicializa as Flags e Variáveis da rodada inicial */
         envia_ou_recebe     = ESTADO_RECEBE;
         inicio_da_rodada    = 1;
         transmissao_arquivo = 0;
         acabou_rodada       = 0;
         reenvia_mensagem    = 0;
         mensagem_com_erro   = 0;
   
         while (!acabou_rodada){
            /* (1) Servidor ENVIA uma Mensagem 
             * 〖 Listagem das Possibilidades 〗
             * Nome da Mensagem | Descrição
             * -------------------------------------------------------------
             * TESOURO          | Tem Tesouro? (Sim ou Não)
             * ACK              | Acknownlegde
             * NACK             | Not Acknownlegde
             * FIM_RODADA       | Fim da Rodada atual
             * ERRO             | Erro (Espaço Insuficiente)
             * 
             * 〖 Tabela de Envio de Mensagens 〗
             * Mensagem RECEBIDA | Mensagem A ENVIAR 
             * -------------------------------------
             *                                                               */
            if (envia_ou_recebe == ESTADO_ENVIA){
               if (reenvia_mensagem == 1){
                  envia_mensagem(msg_anterior, soquete);
                  envia_ou_recebe = ESTADO_RECEBE;
                  reenvia_mensagem = 0;
               }
               else if (mensagem_com_erro == 1){
                  cria_mensagem(msg_enviar, 0, 0, NACK, NULL);
                  envia_mensagem(msg_enviar, soquete);
                  envia_ou_recebe = ESTADO_RECEBE;
                  mensagem_com_erro = 0;
               }
               else {
                  unsigned char msg_tipo = MSG_TIPO(msg_recebida);
                  
                  if (msg_tipo == MV_BX || msg_tipo == MV_CM 
                     || msg_tipo == MV_EQ || msg_tipo == MV_DI){
                     /* Processa o Movimento
                      * ... */
                     unsigned char tem_tesouro = 0;

                     if (MatrizTabuleiro[coord_jogador.l][coord_jogador.c] == 'X')
                        tem_tesouro = 1;

                     cria_mensagem(msg_enviar, 1, 0, TESOURO, &tem_tesouro);
                     envia_mensagem(msg_enviar, soquete);
                     envia_ou_recebe = ESTADO_RECEBE;
                  }
                  else if (MSG_TIPO(msg_recebida) == ACK){
                     cria_mensagem(msg_enviar, 0, 0, FIM_RODADA, NULL);
                     envia_mensagem(msg_enviar, soquete);
                     envia_ou_recebe = ESTADO_RECEBE;
                  }
                  else if (MSG_TIPO(msg_recebida) == FIM_RODADA){
                     cria_mensagem(msg_enviar, 0, 0, ACK, NULL);
                     envia_mensagem(msg_enviar, soquete);
                     envia_ou_recebe = ESTADO_RECEBE;
                  }
                  else if (MSG_TIPO(msg_recebida) == NACK){
                     reenvia_mensagem = 1;
                  }
               }
            }

            /* (2) Servidor RECEBE uma Mensagem */
            else if (envia_ou_recebe == ESTADO_RECEBE){
               validade = recebe_mensagem(msg_recebida, soquete, TEMPO_TIMEOUT);
               if (validade == MSG_TIMEOUT){
                  if (inicio_da_rodada == 0){
                     reenvia_mensagem = 1;
                     envia_ou_recebe  = ESTADO_ENVIA;
                  }
               }
               else if (validade == MSG_ERRO_CHECK){
                  if (inicio_da_rodada == 1) inicio_da_rodada = 0;
                  mensagem_com_erro = 1;
                  envia_ou_recebe = ESTADO_ENVIA;
               }
               else if (validade == MSG_VALIDA){
                  if (inicio_da_rodada == 1) inicio_da_rodada = 0;
                  envia_ou_recebe = ESTADO_ENVIA;
               }
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