#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#include "jogo.h"
#include "tabuleiro.h"
#include "mensagem.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#define NUM_TESOUROS  8
#define TAM_TABULEIRO 8
#define TEMPO_TIMEOUT 1000
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

   unsigned char envia_ou_recebe;
   unsigned char reenvia_mensagem;
   unsigned char mensagem_com_erro;

   unsigned char sequencia_msg;         
   char buffer[127];
   
   mensagem_t msg_enviar   = (mensagem_t) malloc(132 * sizeof(unsigned char));
   mensagem_t msg_anterior = (mensagem_t) malloc(132 * sizeof(unsigned char));
   mensagem_t msg_recebida = (mensagem_t) malloc(132 * sizeof(unsigned char));

   Tesouro_t* Tesouros = (Tesouro_t*) malloc(NUM_TESOUROS * sizeof(Tesouro_t));
   char** MatrizTabuleiro   = alocaMatriz(TAM_TABULEIRO);
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
               tipo_movimento = MV_CM; break;
            case 'a':
               coord_jogador.c--;
               tipo_movimento = MV_EQ; break;
            case 's':
               coord_jogador.l++;
               tipo_movimento = MV_BX; break;
            case 'd':
               coord_jogador.c++;
               tipo_movimento = MV_DI; break;
            default: break;
         }
         
         /* Corrige as Coordenadas Inválidas! */
         if (coord_jogador.l < 0)              coord_jogador.l = 0;
         if (coord_jogador.l == TAM_TABULEIRO) coord_jogador.l = TAM_TABULEIRO - 1;
         if (coord_jogador.c < 0)              coord_jogador.c = 0;
         if (coord_jogador.c == TAM_TABULEIRO) coord_jogador.c = TAM_TABULEIRO - 1;

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

         FILE* novo_arquivo;
         unsigned int tem_tesouro;
         sequencia_msg = 0;

         while (1){
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
                     tem_tesouro = msg_recebida[5];
                     if (tem_tesouro == 0)
                        printf("        [!] Tesouro Achado em (%d , %d) \n", TAM_TABULEIRO - 1 - coord_jogador.l, coord_jogador.c);
                     else
                        printf("        [~] Tesouro Não Achado...\n");
                     cria_mensagem(msg_enviar, 0, 0, ACK, NULL);
                  }
                  /* (MR02) Mensagem ACK */
                  else if (msg_tipo_cl == ACK){

                  }
                  /* (MR03) Mensagem TAMANHO 
                   * Mensagem a Enviar: ACK */
                  else if (msg_tipo_cl == TAMANHO){
                     /* Verifica se é possível transferir o arquivo... */
                     struct statvfs buf;
                     statvfs("./objetos", &buf);

                     unsigned int tamanho_tesouro = *((unsigned int*) &msg_recebida[5]);

                     if (tamanho_tesouro > (buf.f_bavail * buf.f_bsize)){
                        unsigned char erro_tamanho = ESPACO_INSUFICIENTE;
                        cria_mensagem(msg_enviar, 1, 0, ERRO, &erro_tamanho);
                     }
                     else
                        cria_mensagem(msg_enviar, 0, 0, ACK, NULL);
                  }
                  /* (MR04) Mensagem DADOS 
                   * Mensagem a Enviar: ACK */
                  else if (msg_tipo_cl == DADOS){
                     /* Abre o arquivo a transferir... */
                     char* nome_aqv = (char*) malloc(16* sizeof(char))
                     strcpy(nome_aqv, "./objetos/");
                     strcpy(&nome_aqv[10], &msg_recebida[5]);

                     printf("             [Tesouro] %s\n", nome_aqv);
                     novo_arquivo = fopen(nome_aqv, "w");
                     cria_mensagem(msg_enviar, 0, 0, ACK, NULL);
                  }
                  /* (MR05) Mensagem ARQUIVO (Texto, Imagem e Vídeo) 
                   * Mensagem a Enviar: ACK */
                  else if (msg_tipo_cl == TEXTO || msg_tipo_cl == IMAGEM || msg_tipo_cl == VIDEO){
                     /* Verifica se a sequência está correta...
                      * e adiciona os dados no arquivo... */
                     if (MSG_SEQUENCIA(msg_recebida) == sequencia_msg){
                        fwrite((void*) &msg_recebida[5], MSG_TAMANHO(msg_recebida), 1, novo_arquivo);
                        cria_mensagem(msg_enviar, 0, 0, ACK, NULL);
                        sequencia_msg = (sequencia_msg + 1) %32;
                     }
                     else
                        cria_mensagem(msg_enviar, 0, 0, NACK, NULL);
                  }
                  /* (MR06) Mensagem FIM ARQUIVO 
                   * Mensagem a Enviar: ACK */
                  else if (msg_tipo_cl == FIM_ARQUIVO){
                     /* Fecha o arquivo e exibe na tela... */
                     fclose(novo_arquivo);
                     char* abrir_aqv = (char*) malloc(25 * sizeof(char));
                     strcpy(abrir_aqv, "xdg-open ");
                     strcpy(&abrir_aqv[9], nome_aqv);
                     system(abrir_aqv);
                     cria_mensagem(msg_enviar, 0, 0, ACK, NULL);
                  }
                  /* (MR07) Mensagem FIM RODADA 
                   * Mensagem a Enviar: MOVIMENTO */
                  else if (msg_tipo_cl == FIM_RODADA){
                     break;
                  }
                  /* (MR08) Mensagem NACK */
                  else if (msg_tipo_cl == NACK){
                     copia_mensagem(msg_anterior, msg_enviar);
                  }
                  /* (MR09) Mensagem ERRO */
                  else if (msg_tipo_cl == ERRO){
                     cria_mensagem(msg_enviar, 0, 0, ACK, NULL);
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
      abre_tesouros(Tesouros, NUM_TESOUROS);

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

         Tesouro_t tsr;
         
         unsigned char msg_tipo_sv;
         unsigned char num_tesouro;

         sequencia_msg = 0;
         while (1){
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
                     MatrizTabuleiro[coord_jogador.l][coord_jogador.c] = '*';
                     
                     /* Processa o Movimento */
                     switch(msg_tipo_sv){
                        case MV_BX:
                           coord_jogador.l++; break;
                        case MV_CM:
                           coord_jogador.l--; break;
                        case MV_EQ:
                           coord_jogador.c--; break;
                        case MV_DI:
                           coord_jogador.c++; break;
                        default: break;
                     }
                     /* Corrige Posição do Jogador */
                     if (coord_jogador.l < 0)              coord_jogador.l = 0;
                     if (coord_jogador.l == TAM_TABULEIRO) coord_jogador.l = TAM_TABULEIRO - 1;
                     if (coord_jogador.c < 0)              coord_jogador.c = 0;
                     if (coord_jogador.c == TAM_TABULEIRO) coord_jogador.c = TAM_TABULEIRO - 1;

                     num_tesouro = 0;

                     if (MatrizTabuleiro[coord_jogador.l][coord_jogador.c] >= 49 && 
                         MatrizTabuleiro[coord_jogador.l][coord_jogador.c] <= 56){
                           num_tesouro = MatrizTabuleiro[coord_jogador.l][coord_jogador.c] - 48;
                     }

                     MatrizTabuleiro[coord_jogador.l][coord_jogador.c] = 'P';

                     cria_mensagem(msg_enviar, 1, 0, TESOURO, &num_tesouro);
                  }
                  /* (MR02) Mensagem ACK */
                  else if (msg_tipo_sv == ACK){
                     if (MSG_TIPO(msg_anterior) == TESOURO){
                        tsr = Tesouros[num_tesouro - 1];
                        if (tsr.arq_tesouro == NULL){
                           unsigned char id_erro = SEM_PERMISSAO_ACESSO;
                           cria_mensagem(msg_enviar, 1, 0, ERRO, &id_erro);
                        }
                        else {
                           struct stat buf_dados;
                           stat(tsr.arq_tesouro, &buf_dados);
                           unsigned int tamanho = buf_dados.st_size;
                           cria_mensagem(msg_enviar, sizeof(long), 0, TAMANHO, (unsigned char*) &tamanho);
                        }
                     }
                     else if (MSG_TIPO(msg_anterior) == TAMANHO){
                        cria_mensagem(msg_enviar, 6, 0, DADOS, (unsigned char*) tsr.nome_tesouro);
                     }
                     else if (MSG_TIPO(msg_anterior) == DADOS){
                        sequencia_msg = 0;
                        unsigned char bytes_lidos = fread(buffer, 127, 1, tsr.arq_tesouro);
                        sequencia_msg = (sequencia_msg + 1) % 32;

                        cria_mensagem(msg_enviar, bytes_lidos, sequencia_msg, TEXTO, buffer);
                     }
                     else if (MSG_TIPO(msg_anterior) == TEXTO || MSG_TIPO(msg_anterior) == IMAGEM || MSG_TIPO(msg_anterior) == VIDEO){
                        if (feof(tsr.arq_tesouro)){
                           cria_mensagem(msg_enviar, 0, 0, FIM_ARQUIVO, NULL);
                        }
                        else {
                           unsigned char pos_antes = (unsigned char) ftell(tsr.arq_tesouro);
                           fgets(buffer, 127, tsr.arq_tesouro);
                           unsigned char pos_depois = (unsigned char) ftell(tsr.arq_tesouro);
                           unsigned char bytes_lidos = pos_depois - pos_antes;

                           cria_mensagem(msg_enviar, bytes_lidos, sequencia_msg, TEXTO, buffer);
                           sequencia_msg = (sequencia_msg + 1) % 32;
                        }
                     }
                     else if (MSG_TIPO(msg_anterior) == FIM_ARQUIVO || MSG_TIPO(msg_anterior) == ERRO){
                        cria_mensagem(msg_enviar, 0, 0, FIM_RODADA, NULL);
                     }
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
                     if (msg_recebida[5] == ESPACO_INSUFICIENTE)
                        cria_mensagem(msg_enviar, 0, 0, FIM_RODADA, NULL);
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