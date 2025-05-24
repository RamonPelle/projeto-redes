#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/statvfs.h>

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
/* [Função] jogo_tesouro()
 * Descrição: Recebe um soquete de uma interface de rede e o modo de uso do
 * jogo. A função cria estruturas do jogo, como o a matriz do tabuleiro, as
 * coordenadas do jogador e as flags de uso. Então o laço de execução do jogo
 * começa e o movimento do jogador é registrado. Caso haja um tesouro na posi-
 * ção do jogador, o servidor envia o arquivo do tesouro para o local do cli-
 * ente. O jogo acaba quando todas as posições do tabuleiro foram acessadas. */
/*  - Laço de Transmissão de Mensagens -
 * (1) Cliente -> Servidor: Movimento do Jogador 
 * (2) Servidor -> Cliente: Tem Tesouro ou Não
 * (3) Se Não Tem Tesouro:
 *        Acaba a Transmissão e volta ao passo (1)
 *     Se Tem Tesouro:
 *        Servidor -> Cliente: Tamanho do Arquivo 
 *        Servidor -> Cliente: Dados do Arquivo
 *        Enquanto Não Varrer todo o Arquivo:
 *           Servidor -> Cliente: Parte do Arquivo 
 *        Servidor -> Cliente: Avisa que o Arquivo Acabou
 *        Acaba a Transmissão e volta ao passo (1)        */
void jogo_tesouro(int soquete, Usuario usuario)
{
   /* Variáveis do Jogo */
   PosicaoJogador_t coord_jogador;
   coord_jogador.l = TAM_TABULEIRO - 1;
   coord_jogador.c = 0;

   /* Flags de Controle do Programa */
   int validade;
   unsigned char inicio_da_rodada;
   unsigned char jogo_em_andamento;
   unsigned char transmissao_arquivo;
   unsigned char acabou_rodada;
   unsigned char envia_ou_recebe;
   unsigned char reenvia_mensagem;
   unsigned char mensagem_com_erro;

   mensagem_t msg_enviar   = (mensagem_t) malloc(132 * sizeof(unsigned char));
   mensagem_t msg_anterior = (mensagem_t) malloc(132 * sizeof(unsigned char));
   mensagem_t msg_recebida = (mensagem_t) malloc(132 * sizeof(unsigned char));

   char* caminho_absoluto_tesouros[NUM_TESOUROS];
   char** MatrizTabuleiro = alocaMatriz(TAM_TABULEIRO);
   inicia_tabuleiro(MatrizTabuleiro, TAM_TABULEIRO);

   /* <Código do Cliente>
    * (1) A matriz do Tabuleiro contém as posições já visitadas e também
    * mantém a posição do Jogador.
    * (2) O Cliente deve RECEBER mensagens do tipo ARQUIVO (Texto, Imagem
    * e Vídeo), não enviá-las. */
   if (usuario == CLIENTE){
      MatrizTabuleiro[coord_jogador.l][coord_jogador.c] = 'P';
      
      char movimento;

      jogo_em_andamento = 1;
      while (jogo_em_andamento){
         imprime_tabuleiro(MatrizTabuleiro, TAM_TABULEIRO);

         printf("Realize o seu movimento, astronauta: ");
         scanf("%c", &movimento);
         getchar();

         MatrizTabuleiro[coord_jogador.l][coord_jogador.c] = '*';

         unsigned char tipo_movimento = 0;

         switch(movimento){
            case 'w':
               coord_jogador.l--;
               tipo_movimento = MV_CM;
               break;
            case 'a':
               coord_jogador.c--;
               tipo_movimento = MV_EQ;
               break;
            case 's':
               coord_jogador.l++;
               tipo_movimento = MV_BX;
               break;
            case 'd':
               coord_jogador.c++;
               tipo_movimento = MV_DI;
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

         /* Inicializa as Flags e Variáveis */
         transmissao_arquivo = 0;
         acabou_rodada       = 0;
         reenvia_mensagem    = 0;
         mensagem_com_erro   = 0;

         cria_mensagem(msg_enviar, 0, 0, tipo_movimento, NULL);
         envia_mensagem(msg_enviar, soquete);
         copia_mensagem(msg_enviar, msg_anterior);
         envia_ou_recebe     = ESTADO_RECEBE;

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
             * -------------------------------------------------------------
             * 
             * 〖 Tabela de Envio de Mensagens 〗
             * RECEBIDA    | A ENVIAR 
             * -------------------------------------
             * TESOURO     | ACK
             * ACK         | - Encerra Rodada -
             * TAMANHO     | ACK, se HÁ ESPAÇO
             *             | ERRO, se NÃO HÁ ESPAÇO
             * DADOS       | ACK
             * TEXTO       | ACK
             * IMAGEM      | ACK
             * VIDEO       | ACK
             * FIM_ARQUIVO | ACK
             * FIM_RODADA  | FIM_RODADA
             * NACK        | Última Mensagem Enviada
             * ERRO        | ACK                                           
             * -------------------------------------                         */
            if (envia_ou_recebe == ESTADO_ENVIA){
               /* (ST01) Tempo de Timeout Atingido... */
               if (reenvia_mensagem == 1){
                  copia_mensagem(msg_anterior, msg_enviar);
                  reenvia_mensagem = 0;
               }
               /* (ST02) Mensagem Recebida contém Erro... */
               else if (mensagem_com_erro == 1){
                  cria_mensagem(msg_enviar, 0, 0, NACK, NULL);
                  copia_mensagem(msg_enviar, msg_anterior);
                  mensagem_com_erro = 0;
               }
               /* (ST03) Mensagem Recebida Confirmada! */
               else {
                  unsigned char msg_tipo_cl = MSG_TIPO(msg_recebida);

                  /* (MR01) Mensagem TESOURO */
                  if (msg_tipo_cl == TESOURO){
                     if (msg_recebida[5] == 0){
                        printf("\nTESOURO NAO FOI ENCONTRADO...\n");
                     }
                     else {
                        printf("\nTESOURO ENCONTRADO!!!\n");
                     }

                     cria_mensagem(msg_enviar, 0, 0, ACK, NULL);
                  }
                  /* (MR02) Mensagem ACK */
                  else if (msg_tipo_cl == ACK){

                  }
                  /* (MR03) Mensagem TAMANHO */
                  else if (msg_tipo_cl == TAMANHO){

                  }
                  /* (MR04) Mensagem DADOS */
                  else if (msg_tipo_cl == DADOS){

                  }
                  /* (MR05) Mensagem ARQUIVO (Texto, Imagem e Vídeo) */
                  else if (msg_tipo_cl == TEXTO || msg_tipo_cl == IMAGEM || msg_tipo_cl == VIDEO){

                  }
                  /* (MR06) Mensagem FIM ARQUIVO */
                  else if (msg_tipo_cl == FIM_ARQUIVO){

                  }
                  /* (MR07) Mensagem FIM RODADA */
                  else if (msg_tipo_cl == FIM_RODADA){

                  }
                  /* (MR08) Mensagem NACK */
                  else if (msg_tipo_cl == NACK){
                     copia_mensagem(msg_anterior, msg_enviar);
                  }
                  /* (MR09) Mensagem ERRO */
                  else if (msg_tipo_cl == ERRO){

                  }
               }

               envia_mensagem(msg_anterior, soquete);
               copia_mensagem(msg_enviar, msg_anterior);
               envia_ou_recebe = ESTADO_RECEBE;
            }

            /* (2) Cliente RECEBE uma Mensagem */
            else if (envia_ou_recebe == ESTADO_RECEBE){
               validade = recebe_mensagem(msg_recebida, soquete, TEMPO_TIMEOUT);
               
               /* (ST01) Tempo de Timeout Atingido... */
               if (validade == MSG_TIMEOUT || validade == MSG_INVALIDA){
                  reenvia_mensagem = 1;
               }
               /* (ST02) Mensagem Recebida contém Erro... */
               else if (validade == MSG_ERRO_CHECK){
                  mensagem_com_erro = 1;
               }

               envia_ou_recebe = ESTADO_ENVIA;
            }
         }
         
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

   /* <Código do Servidor>
    * (1) A matriz do Tabuleiro contém as localizações dos tesouros.
    * (2) O Cliente deve ENVIAR mensagens do tipo ARQUIVO (Texto, Imagem
    * e Vídeo), não recebê-las. */
   else if (usuario == SERVIDOR){
      /* Gera os Tesouros no Tabuleiro */
      gera_tesouros(MatrizTabuleiro, TAM_TABULEIRO, NUM_TESOUROS);

      jogo_em_andamento = 1;
      while (jogo_em_andamento){
         imprime_tabuleiro(MatrizTabuleiro, TAM_TABULEIRO);

         /*           - = Transmissão de Mensagens = -            */

         /* Inicializa as Flags e Variáveis */
         transmissao_arquivo = 0;
         acabou_rodada       = 0;
         reenvia_mensagem    = 0;
         mensagem_com_erro   = 0;
         
         envia_ou_recebe     = ESTADO_RECEBE;

         unsigned char msg_tipo_sv;

         while (!acabou_rodada){
            /* (1) Servidor ENVIA uma Mensagem 
             * 〖 Listagem das Possibilidades 〗
             * Nome da Mensagem | Descrição
             * ---------------------------------------------
             * TESOURO          | Tem Tesouro? (Sim ou Não)
             * ACK              | Acknownlegde
             * NACK             | Not Acknownlegde
             * FIM_RODADA       | Fim da Rodada atual
             * ERRO             | Erro (Espaço Insuficiente)
             * ---------------------------------------------
             * 
             * 〖 Tabela de Envio de Mensagens 〗
             * RECEBIDA   | Mensagem A ENVIAR 
             * ----------------------------------------------
             * MOVIMENTO  | TESOURO                                                
             * ACK        | TAMANHO, se TEM TESOURO
             *            | DADOS, se HÁ ESPAÇO
             *            | TEXTO, se RESTA ARQUIVO
             *            | IMAGEM, se RESTA ARQUIVO
             *            | VIDEO, se RESTA ARQUIVO
             *            | ERRO, se NÃO HÁ ESPAÇO
             *            | FIM_ARQUIVO, se NÃO RESTA ARQUIVO
             *            | FIM_RODADA, se ACABA RODADA
             * FIM_RODADA | ACK
             * NACK       | Última Mensagem Enviada
             * ERRO       | FIM_RODADA         
             * ---------------------------------------------- */
            if (envia_ou_recebe == ESTADO_ENVIA){
               /* (ST01) Tempo de Timeout Atingido... */
               if (reenvia_mensagem == 1){
                  copia_mensagem(msg_anterior, msg_enviar);
                  reenvia_mensagem = 0;
               }
               /* (ST02) Mensagem Recebida contém Erro... */
               else if (mensagem_com_erro == 1){
                  cria_mensagem(msg_enviar, 0, 0, NACK, NULL);
                  mensagem_com_erro = 0;
               }
               /* (ST03) Mensagem Recebida Confirmada! */
               else {
                  msg_tipo_sv = MSG_TIPO(msg_recebida);
                  
                  /* (MR01) Mensagem MOVIMENTO */
                  if (msg_tipo_sv == MV_BX || msg_tipo_sv == MV_CM || msg_tipo_sv == MV_EQ || msg_tipo_sv == MV_DI){
                     /* Processa o Movimento */
                     switch(msg_tipo_sv){
                        case MV_BX:
                           coord_jogador.l++;
                           break;
                        case MV_CM:
                           coord_jogador.l--;
                           break;
                        case MV_EQ:
                           coord_jogador.c--;
                           break;
                        case MV_DI:
                           coord_jogador.c++;
                           break;
                        default:
                           break;
                     }
                     /* Corrige Posição do Jogador */
                     if (coord_jogador.l < 0)
                        coord_jogador.l = 0;
                     if (coord_jogador.l == TAM_TABULEIRO)
                        coord_jogador.l = TAM_TABULEIRO - 1;
                     if (coord_jogador.c < 0)
                        coord_jogador.c = 0;
                     if (coord_jogador.c == TAM_TABULEIRO)
                        coord_jogador.c = TAM_TABULEIRO - 1;

                     unsigned char tem_tesouro = 0;

                     if (MatrizTabuleiro[coord_jogador.l][coord_jogador.c] == 'X')
                        tem_tesouro = 1;

                     cria_mensagem(msg_enviar, 1, 0, TESOURO, &tem_tesouro);
                  }
                  /* (MR02) Mensagem ACK */
                  else if (msg_tipo_sv == ACK){
                     cria_mensagem(msg_enviar, 0, 0, FIM_RODADA, NULL);
                  }
                  /* (MR03) Mensagem FIM_RODADA */
                  else if (msg_tipo_sv == FIM_RODADA){
                     cria_mensagem(msg_enviar, 0, 0, ACK, NULL);
                  }
                  /* (MR04) Mensagem NACK */
                  else if (msg_tipo_sv == NACK){
                     copia_mensagem(msg_anterior, msg_enviar);
                  }
                  /* (MR05) Mensagem ERRO */
                  else if (msg_tipo_sv == ERRO){

                  }
               }

               envia_mensagem(msg_enviar, soquete);
               copia_mensagem(msg_enviar, msg_anterior);
               envia_ou_recebe = ESTADO_RECEBE;
            }

            /* (2) Servidor RECEBE uma Mensagem */
            else if (envia_ou_recebe == ESTADO_RECEBE){
               validade = recebe_mensagem(msg_recebida, soquete, TEMPO_TIMEOUT);

               /* (ST01) Tempo de Timeout Atingido... */
               if (validade == MSG_TIMEOUT || validade == MSG_INVALIDA){
                  reenvia_mensagem = 1;
               }
               /* (ST02) Mensagem Recebida contém Erro... */
               else if (validade == MSG_ERRO_CHECK){
                  mensagem_com_erro = 1;
               }
               /* (ST03) Mensagem Recebida Confirmada! */
               else if (validade == MSG_VALIDA){

               }

               envia_ou_recebe = ESTADO_ENVIA;
            }
         }

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
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */