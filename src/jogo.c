#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#include "jogo.h"
#include "tabuleiro.h"
#include "mensagem.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#define NUM_TESOUROS  8
#define TAM_TABULEIRO 8
#define TEMPO_TIMEOUT 2000
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
   unsigned char envia_ou_recebe;
   unsigned char reenvia_mensagem;
   unsigned char mensagem_com_erro;
   unsigned char mensagem_invalida = 0;

   char* caminho_aqv = (char*) malloc(16 * sizeof(char));
   unsigned char sequencia_msg;         
   unsigned char buffer[127];
   
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
      imprime_tabuleiro(MatrizTabuleiro, TAM_TABULEIRO);

      while (1){

         /* Leitura do movimento do Jogador.
          * 'w': Cima , 'a': Esquerda, 's': Baixo, 'd': Direita */
         char movimento;
         printf("Realize o seu movimento, astronauta (w/a/s/d): ");
         scanf("%c", &movimento);
         getchar();

         /* Marca que o Jogador já passou por aquela posição. */
         MatrizTabuleiro[coord_jogador.l][coord_jogador.c] = '*';

         /* Altera as coordenadas do Jogador baseado no movimento. */
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
         
         /* Corrige as coordenadas, caso elas tenham 
          * passado do limite do tabuleiro. */
         if (coord_jogador.l < 0)              coord_jogador.l = 0;
         if (coord_jogador.l == TAM_TABULEIRO) coord_jogador.l = TAM_TABULEIRO - 1;
         if (coord_jogador.c < 0)              coord_jogador.c = 0;
         if (coord_jogador.c == TAM_TABULEIRO) coord_jogador.c = TAM_TABULEIRO - 1;

         /* Atualiza a posição do Jogador. */
         MatrizTabuleiro[coord_jogador.l][coord_jogador.c] = 'P';

         /* Imprime o Tabuleiro na tela. */
         sleep(1);
         system("clear");
         imprime_tabuleiro(MatrizTabuleiro, TAM_TABULEIRO);

         /* - - - - - - = = = = = = = = = = = = = = = = - - - - - - */
         /*             - = Transmissão de Mensagens  = -           */
         /* - - - - - - = = = = = = = = = = = = = = = = - - - - - - */
         /* Inicializa as Flags e Variáveis */
         reenvia_mensagem  = 0;
         mensagem_com_erro = 0;

         cria_mensagem(msg_enviar, 0, 0, tipo_movimento, NULL);
         envia_mensagem(msg_enviar, soquete);
         copia_mensagem(msg_enviar, msg_anterior);

         envia_ou_recebe = ESTADO_RECEBE;

         /* Flags de Controle específicas do Cliente... */
         FILE* novo_arquivo;
         unsigned int tem_tesouro;
         unsigned int tesouro_processado;
         unsigned char sequencia_atual;
         unsigned char acabou_rodada;
         unsigned char ultima_msg_nack;

         tem_tesouro = 0;
         acabou_rodada = 0;
         sequencia_msg = 0;
         tesouro_processado = 0;
         ultima_msg_nack = 1;
         
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
                  printf("        [D] Reenviando Mensagem %d.\n", MSG_TIPO(msg_anterior));
                  copia_mensagem(msg_anterior, msg_enviar);
                  reenvia_mensagem = 0;
               }


               /* (ST02) Mensagem Recebida contém Erro... */
               else if (mensagem_com_erro == 1){
                  printf("        [D] Mensagem com Erro.\n");
                  cria_mensagem(msg_enviar, 0, 0, NACK, NULL);
                  mensagem_com_erro = 0;
               }


               /* (ST03) Mensagem Recebida Confirmada! */
               else if (!mensagem_invalida) {
                  unsigned char msg_tipo_cl = MSG_TIPO(msg_recebida);


                  /* (MR01) Mensagem OK */
                  if (msg_tipo_cl == OK){
                     printf("        [D] Recebeu OK.\n");

                     if (!tesouro_processado){
                        tem_tesouro = msg_recebida[5];

                        if (tem_tesouro == 0){
                           printf("        [~] Tesouro Não Achado...\n");
                           acabou_rodada = 1;
                        }
                        else
                           printf("        [!] Tesouro Achado em (%d , %d)!\n", TAM_TABULEIRO - 1 - coord_jogador.l, coord_jogador.c);

                        tesouro_processado = 1;
                     }

                     printf("        [D] Envia ACK-OK.\n");
                     cria_mensagem(msg_enviar, 0, 0, ACK, NULL);
                  }


                  /* (MR02) Mensagem TAMANHO 
                   * Mensagem a Enviar: ACK */
                  else if (msg_tipo_cl == TAMANHO){
                     printf("        [D] Recebeu TAMANHO.\n");


                     /* Verifica se é possível transferir o arquivo... */
                     struct statvfs buf;
                     statvfs("./objetos", &buf);

                     unsigned int tamanho_tesouro = *((unsigned int*) &msg_recebida[5]);
                     printf("        [B] Tamanho do Tesouro: %u B\n", tamanho_tesouro);

                     if (tamanho_tesouro > (buf.f_bavail * buf.f_bsize)){
                        unsigned char erro_tamanho = ESPACO_INSUFICIENTE;

                        printf("        [D] Envia ERRO.\n");
                        cria_mensagem(msg_enviar, 1, 0, ERRO, &erro_tamanho);
                     }
                     else {
                        printf("        [D] Envia ACK-TAMANHO.\n");
                        cria_mensagem(msg_enviar, 0, 0, ACK, NULL);
                     }
                  }


                  /* (MR03) Mensagem DADOS 
                   * Mensagem a Enviar: ACK */
                  else if (msg_tipo_cl == DADOS){
                     /* Abre o arquivo a transferir... */
                     printf("        [N] Nome do Tesouro: %s\n", &msg_recebida[5]);

                     strcpy(caminho_aqv, "./tesouro/");
                     strcpy(&caminho_aqv[10], (char*) &msg_recebida[5]);

                     novo_arquivo = fopen(caminho_aqv, "wb");
                     cria_mensagem(msg_enviar, 0, 0, ACK, NULL);
                  }


                  /* (MR04) Mensagem ARQUIVO (Texto, Imagem e Vídeo) 
                   * Mensagem a Enviar: ACK */
                  else if (msg_tipo_cl == TEXTO || msg_tipo_cl == IMAGEM || msg_tipo_cl == VIDEO){

                     printf("        [D] Recebeu ARQUIVO[%d].\n", MSG_SEQUENCIA(msg_recebida));
                     /* Verifica se a sequência está correta...
                      * e adiciona os dados no arquivo... */
                     if (MSG_SEQUENCIA(msg_recebida) == sequencia_atual){

                        fwrite((void*) &msg_recebida[5], MSG_TAMANHO(msg_recebida), 1, novo_arquivo);

                        printf("        [D] Envia ACK-ARQUIVO.\n");

                        cria_mensagem(msg_enviar, 0, 0, ACK, NULL);

                        sequencia_atual = (sequencia_atual + 1) % 32;
                     }
                     
                     else if (MSG_SEQUENCIA(msg_recebida) < sequencia_atual){
                        printf("        [D] Envia ACK-ARQUIVO.\n");
                        cria_mensagem(msg_enviar, 0, 0, ACK, NULL);
                     }

                  }


                  /* (MR05) Mensagem FIM_ARQUIVO 
                   * Mensagem a Enviar: MOVIMENTO */
                  else if (msg_tipo_cl == FIM_ARQUIVO){
                     printf("        [D] Recebeu FIM-ARQUIVO.\n");

                     /* Fecha o arquivo e exibe na tela... */
                     if (tem_tesouro){
                        fclose(novo_arquivo);
                        printf("        [Y] Tesouro Coletado com Sucesso!\n");

                        char* abrir_aqv = (char*) malloc(25 * sizeof(char));
                        strcpy(abrir_aqv, "xdg-open ");
                        strcpy(&abrir_aqv[9], caminho_aqv);
                        //system(abrir_aqv);

                        free(abrir_aqv);

                        acabou_rodada = 1;
                     }

                     if (acabou_rodada){
                        printf("        [D] Acabou RODADA.\n");
                        break;
                     }
                  }


                  /* (MR06) Mensagem NACK */
                  else if (msg_tipo_cl == NACK){
                     printf("        [D] Recebeu NACK.\n");
                     if (ultima_msg_nack)
                        cria_mensagem(msg_enviar, 0, 0, NACK, NULL);
                     else 
                        copia_mensagem(msg_anterior, msg_enviar);
                  }


                  /* (MR07) Mensagem ERRO */
                  else if (msg_tipo_cl == ERRO){
                     printf("        [D] Recebeu ERRO.\n");

                     printf("        [E] Erro. Tesouro Não Coletado.\n");
                     cria_mensagem(msg_enviar, 0, 0, ACK, NULL);

                     printf("        [D] Envia ACK-ERRO.\n");
                  }
               }
               
               
               ultima_msg_nack = (MSG_TIPO(msg_enviar) == NACK) ? 1 : 0;
               if (!ultima_msg_nack) copia_mensagem(msg_enviar, msg_anterior);  
               
               if (!mensagem_invalida) {
                  envia_mensagem(msg_enviar, soquete);
               }
               mensagem_invalida = 0;
               envia_ou_recebe = ESTADO_RECEBE;
            }

            /* (2) Cliente RECEBE uma Mensagem */
            else if (envia_ou_recebe == ESTADO_RECEBE){
               validade = recebe_mensagem(msg_recebida, soquete, TEMPO_TIMEOUT);
               
               /* (ST01) Tempo de Timeout Atingido... */
               if (validade == MSG_TIMEOUT){
                  reenvia_mensagem = 1;
               }
               /* (ST02) Mensagem Recebida contém Erro... */
               else if (validade == MSG_ERRO_CHECK){
                  mensagem_com_erro = 1;
               }
               
               /* (ST03) Mensagem Recebida está inválida... */
               else if (validade == MSG_INVALIDA) {
                  mensagem_invalida = 1;
               }
               
               envia_ou_recebe = ESTADO_ENVIA;
            }
         }
         
         /* Fim de jogo! Parabéns por completar o nosso jogo! */
         if (fim_de_jogo(MatrizTabuleiro, TAM_TABULEIRO)) break;
      }
      
      /* Jogou Terminou! */
      imprime_tabuleiro(MatrizTabuleiro, TAM_TABULEIRO);
      printf("              [Fim de Jogo]            \n");
      printf("           Obrigado por Jogar! =D      \n");
   }

   /* <Código do Servidor>
    * (1) A matriz do Tabuleiro contém as localizações dos tesouros.
    * (2) O Cliente deve ENVIAR mensagens do tipo ARQUIVO (Texto, Imagem
    * e Vídeo), não recebê-las. */
   else if (usuario == SERVIDOR){

      /* Gera os Tesouros no Tabuleiro. */
      gera_tesouros(MatrizTabuleiro, TAM_TABULEIRO, NUM_TESOUROS);

      /* Abre os arquivos dos Tesouros, guardando em um vetor de Arquivos. */
      abre_tesouros(Tesouros, NUM_TESOUROS);

      while (1){
         imprime_tabuleiro(MatrizTabuleiro, TAM_TABULEIRO);

         /* - - - - - - = = = = = = = = = = = = = = = = - - - - - - */
         /*             - = Transmissão de Mensagens  = -           */
         /* - - - - - - = = = = = = = = = = = = = = = = - - - - - - */
         /* Inicializa as Flags e Variáveis */
         reenvia_mensagem    = 0;
         mensagem_com_erro   = 0;
         
         envia_ou_recebe     = ESTADO_RECEBE;
         sequencia_msg       = 0;
         
         /* Flags de Controle específicas do Servidor... */
         Tesouro_t tsr;
         unsigned char msg_tipo_sv;
         unsigned char num_tesouro;
         unsigned char bytes_lidos;
         unsigned char movimento_processado = 0;
         unsigned char ultima_msg_nack = 0;

         cria_mensagem(msg_anterior, 0, 0, LIVRE01, NULL);

         while (1){
            /* (1) Servidor ENVIA uma Mensagem 
             * 〖 Listagem das Possibilidades 〗
             * Nome da Mensagem | Descrição
             * ---------------------------------------------
             * TESOURO          | Tem Tesouro? (Sim ou Não)
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
             * NACK       | Última Mensagem Enviada
             * ERRO       | FIM_ARQUIVO        
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
               else if (!mensagem_invalida){
                  msg_tipo_sv = MSG_TIPO(msg_recebida);
                  

                  /* (MR01) Mensagem MOVIMENTO */
                  if (msg_tipo_sv == MV_BX || msg_tipo_sv == MV_CM || msg_tipo_sv == MV_EQ || msg_tipo_sv == MV_DI){
                     if (!movimento_processado){
                        int coord_x_anterior = TAM_TABULEIRO - 1 - coord_jogador.l;
                        int coord_y_anterior = coord_jogador.c;

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

                        if (MatrizTabuleiro[coord_jogador.l][coord_jogador.c] >= '1' && 
                           MatrizTabuleiro[coord_jogador.l][coord_jogador.c] <= '8'){
                              num_tesouro = MatrizTabuleiro[coord_jogador.l][coord_jogador.c] - 48;
                        }

                        MatrizTabuleiro[coord_jogador.l][coord_jogador.c] = 'P';

                        sleep(1);
                        system("clear");
                        imprime_tabuleiro(MatrizTabuleiro, TAM_TABULEIRO);

                        printf("        [M] Movimento do Jogador: (%d, %d) -> ", coord_x_anterior, coord_y_anterior);
                        printf("(%d, %d)\n", TAM_TABULEIRO - 1 - coord_jogador.l, coord_jogador.c);

                        if (num_tesouro == 0)
                           printf("        [~] Tesouro Não Achado....\n");
                        else if (num_tesouro > 0)
                           printf("        [!] Tesouro Achado em (%d, %d)!\n", TAM_TABULEIRO - 1 - coord_jogador.l, coord_jogador.c);
                        
                        movimento_processado = 1;
                     }

                     cria_mensagem(msg_enviar, 1, 0, OK, &num_tesouro);
                  }


                  /* (MR02) Mensagem ACK */
                  else if (msg_tipo_sv == ACK){

                     /* (ACK01) Mandou TESOURO -> Envia TAMANHO ou FIM_RODADA */
                     if (MSG_TIPO(msg_anterior) == OK){
                        /* Tesouro encontrado -> Envia o Tamanho do Tesouro. */
                        if (num_tesouro != 0){       
                           tsr = Tesouros[num_tesouro - 1];
                           /* Não foi possível abrir o arquivo -> Erro. */
                           if (tsr.arq_tesouro == NULL){
                              printf("        [E] Não foi possível abrir o tesouro %d.\n", num_tesouro);
                              unsigned char id_erro = SEM_PERMISSAO_ACESSO;
                              cria_mensagem(msg_enviar, 1, 0, ERRO, &id_erro);
                           }
                           /* Arquivo está aberto -> Envia o Tamanho. */
                           else {
                              printf("        [N] Nome do Tesouro: %s\n", tsr.nome_tesouro);

                              struct stat buf_dados;
                              strcpy(caminho_aqv, "./objetos/");
                              strcpy(&caminho_aqv[10], tsr.nome_tesouro);

                              stat(caminho_aqv, &buf_dados);
                              unsigned int tamanho = buf_dados.st_size;

                              printf("        [B] Tamanho do Tesouro: %u B\n", tamanho);
                              cria_mensagem(msg_enviar, sizeof(long), 0, TAMANHO, (unsigned char*) &tamanho);
                           }
                        }

                        /* Tesouro NÃO encontrado -> Acaba Rodada. */
                        else { 
                           cria_mensagem(msg_enviar, 0, 0, FIM_ARQUIVO, NULL);
                           movimento_processado = 0;
                        }
                     }

                     /* (ACK02) Mandou TAMANHO -> Envia DADOS */
                     else if (MSG_TIPO(msg_anterior) == TAMANHO){
                        cria_mensagem(msg_enviar, 6, 0, DADOS, (unsigned char*) tsr.nome_tesouro);
                     }

                     /* (ACK03) Mandou DADOS -> Envia TEXTO ou IMAGEM ou VIDEO */
                     else if (MSG_TIPO(msg_anterior) == DADOS){
                        sequencia_msg = 0;

                        bytes_lidos = fread(buffer, 1, 127, tsr.arq_tesouro);
                        if (bytes_lidos > 0){
                           cria_mensagem(msg_enviar, bytes_lidos, sequencia_msg, TEXTO, buffer);
                        }
                        else {
                           cria_mensagem(msg_enviar, 0, 0, FIM_ARQUIVO, NULL);
                           movimento_processado = 0;
                        }
                     }

                     /* (ACK04) Mandou TEXTO, IMAGEM ou VIDEO -> Envia TEXTO, IMAGEM, VIDEO ou FIM_ARQUIVO */
                     else if (MSG_TIPO(msg_anterior) == TEXTO || MSG_TIPO(msg_anterior) == IMAGEM || MSG_TIPO(msg_anterior) == VIDEO){
                        /* Atualiza o número da sequência do arquivo, para controle de fluxo. */
                        sequencia_msg = (sequencia_msg + 1) % 32;

                        /* Se o Arquivo foi completamente lido, avisa que o Arquivo acabou.
                         * Se não, continua lendo do arquivo. */
                        bytes_lidos = fread(buffer, 1, 127, tsr.arq_tesouro);
                        if (bytes_lidos > 0){
                           cria_mensagem(msg_enviar, bytes_lidos, sequencia_msg, TEXTO, buffer);
                        }
                        else {
                           cria_mensagem(msg_enviar, 0, 0, FIM_ARQUIVO, NULL);
                           movimento_processado = 0;
                        }
                     }

                     /* (ACK05) Mandou ERRO -> Envia FIM_ARQUIVO */
                     else if (MSG_TIPO(msg_anterior) == ERRO){
                        cria_mensagem(msg_enviar, 0, 0, FIM_ARQUIVO, NULL);
                        movimento_processado = 0;
                     }
                  }


                  /* (MR03) Mensagem NACK */
                  else if (msg_tipo_sv == NACK){
                     if (ultima_msg_nack)
                        cria_mensagem(msg_enviar, 0, 0, NACK, NULL);
                     else
                        copia_mensagem(msg_anterior, msg_enviar);
                  }


                  /* (MR04) Mensagem ERRO */
                  else if (msg_tipo_sv == ERRO){
                     
                     if (msg_recebida[5] == ESPACO_INSUFICIENTE){
                        printf("        [E] Espaço Insuficiente para o tesouro.\n");
                        cria_mensagem(msg_enviar, 0, 0, FIM_ARQUIVO, NULL);
                        movimento_processado = 0;
                     }
                  }
               }

               ultima_msg_nack = (MSG_TIPO(msg_enviar) == NACK) ? 1 : 0;
               if (!ultima_msg_nack) copia_mensagem(msg_enviar, msg_anterior);
               
               if (!mensagem_invalida) {
                  envia_mensagem(msg_enviar, soquete);
               }

               mensagem_invalida = 0;
               envia_ou_recebe = ESTADO_RECEBE;
            }

            /* (2) Servidor RECEBE uma Mensagem */
            else if (envia_ou_recebe == ESTADO_RECEBE){
               validade = recebe_mensagem(msg_recebida, soquete, TEMPO_TIMEOUT);

               /* (ST01) Tempo de Timeout Atingido... */
               if (validade == MSG_TIMEOUT){
                  reenvia_mensagem = 1;
               }
               /* (ST02) Mensagem Recebida contém Erro... */
               else if (validade == MSG_ERRO_CHECK){
                  mensagem_com_erro = 1;
               }

               /* (ST03) Mensagem Recebida está inválida... */
               else if (validade == MSG_INVALIDA) {
                  mensagem_invalida = 1;
               }

               envia_ou_recebe = ESTADO_ENVIA;
            }
         }

         /* Fim de jogo! Parabéns por completar o nosso jogo! */
         if (fim_de_jogo(MatrizTabuleiro, TAM_TABULEIRO)) break;
         
         sleep(1);
         system("clear");
      }

      /* Jogo Terminou! */
      system("clear");
      imprime_tabuleiro(MatrizTabuleiro, TAM_TABULEIRO);
      printf("              [Fim de Jogo]            \n");
   }

   /* Libera a Memória das Estruturas */
   liberaMatriz(MatrizTabuleiro, TAM_TABULEIRO);
   free(msg_enviar);
   free(msg_anterior);
   free(msg_recebida);
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */