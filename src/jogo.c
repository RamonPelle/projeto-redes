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
#define NUM_TESOUROS    8
#define TAM_TABULEIRO   8
#define TIMEOUT_INICIAL 1000
#define DEBUG           1
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* - - - - - - - - - - - - - ESTADOS DO CLIENTE  - - - - - - - - - - - - - - */
#define CL_PROCESSA_OK(estados)        estados[0]
#define CL_PROCESSA_BOLO(estados)      estados[1]
#define CL_PROCESSA_DADOS(estados)     estados[2]
#define CL_PROCESSA_ARQUIVO(estados)   estados[3]
#define CL_PROCESSA_FIM(estados)       estados[4]
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* - - - - - - - - - - - - - ESTADOS DO SERVIDOR - - - - - - - - - - - - - - */
#define SV_PROCESSA_MOVIMENTO(estados) estados[0]
#define SV_MONTA_TAMANHO_SV(estados)   estados[1]
#define SV_MONTA_DADOS(estados)        estados[2]
#define SV_MONTA_ARQUIVO(estados)      estados[3]
#define SV_MONTA_FIM(estados)          estados[4]
#define SV_MONTA_ERRO(estados)         estados[5]
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

unsigned char d = DEBUG;
unsigned char num_timeouts = 0;
int tempo_timeout = TIMEOUT_INICIAL;
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
   unsigned char estados[6];

   char* caminho_aqv = (char*) malloc(16 * sizeof(char));
   unsigned char sequencia_atual;
   unsigned char sequencia_anterior;         
   unsigned char buffer[127];
   
   mensagem_t msg_enviar   = (mensagem_t) malloc(132 * sizeof(unsigned char));
   mensagem_t msg_anterior = (mensagem_t) malloc(132 * sizeof(unsigned char));
   mensagem_t msg_recebida = (mensagem_t) malloc(132 * sizeof(unsigned char));

   Tesouro_t* Tesouros     = (Tesouro_t*) malloc(NUM_TESOUROS * sizeof(Tesouro_t));
   char** MatrizTabuleiro  = alocaMatriz(TAM_TABULEIRO);
   inicia_tabuleiro(MatrizTabuleiro, TAM_TABULEIRO);

   /* <Código do Cliente>
    * (1) A matriz do Tabuleiro contém as posições já visitadas e também
    * mantém a posição do Jogador.
    * (2) O Cliente deve RECEBER mensagens do tipo ARQUIVO (Texto, Imagem
    * e Vídeo), não enviá-las. */
   if (usuario == CLIENTE){
      MatrizTabuleiro[coord_jogador.l][coord_jogador.c] = 'P';
      system("clear");
      imprime_tabuleiro(MatrizTabuleiro, TAM_TABULEIRO);

      while (1){

         /* Leitura do movimento do Jogador.
          * 'w': Cima , 'a': Esquerda, 's': Baixo, 'd': Direita */
         char movimento;
         char buffer_leitura[256];
         printf("Realize o seu movimento, astronauta (w/a/s/d): ");

         do {
            fgets(buffer_leitura, 256, stdin);

            movimento = buffer_leitura[0];

            if (strcmp(buffer_leitura, "w\n") != 0 && strcmp(buffer_leitura, "a\n") != 0 
               && strcmp(buffer_leitura, "s\n") != 0 && strcmp(buffer_leitura, "d\n") != 0)
               printf("Comando inválido... tente novamente (w/a/s/d): ");
         } while (strcmp(buffer_leitura, "w\n") != 0 && strcmp(buffer_leitura, "a\n") != 0 
                 && strcmp(buffer_leitura, "s\n") != 0 && strcmp(buffer_leitura, "d\n") != 0);

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
         system("clear");
         imprime_tabuleiro(MatrizTabuleiro, TAM_TABULEIRO);

         /* - - - - - - = = = = = = = = = = = = = = = = - - - - - - */
         /*             - = Transmissão de Mensagens  = -           */
         /* - - - - - - = = = = = = = = = = = = = = = = - - - - - - */

         /* Inicializa as Flags e Variáveis */
         cria_mensagem(msg_enviar, 0, 0, tipo_movimento, NULL);
         envia_mensagem(msg_enviar, soquete);
         copia_mensagem(msg_enviar, msg_anterior);

         envia_ou_recebe = ESTADO_RECEBE;

         estados[0] = 1;
         estados[1] = estados[2] = estados[3] = estados[4] = estados[5] = 0;

         /* Flags de Controle específicas do Cliente... */
         FILE* novo_arquivo;
         unsigned int tem_tesouro;
         unsigned char erro_tamanho;
         sequencia_atual = 0;

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
             * NACK        | Última Mensagem Enviada
             * ERRO        | ACK                                           
             * -------------------------------------                         */
            if (envia_ou_recebe == ESTADO_ENVIA){

               /* Parte I: Processamento da Mensagem */
               /* Faz algum tipo de ação baseado na Mensagem Recebida. */

               /* (MR01) Mensagem OK */
               if (CL_PROCESSA_OK(estados) && MSG_TIPO(msg_recebida) == OK){

                  tem_tesouro = msg_recebida[5];

                  if (tem_tesouro == 0)
                     printf("        [~] Tesouro Não Achado...\n");
                  else
                     printf("        [!] Tesouro Achado em (%d , %d)!\n", TAM_TABULEIRO - 1 - coord_jogador.l, coord_jogador.c);
                  
                  CL_PROCESSA_OK(estados)   = 0;
                  CL_PROCESSA_BOLO(estados) = 1;
               }

               /* (MR02) Mensagem TAMANHO 
                  * Mensagem a Enviar: ACK */
               else if (CL_PROCESSA_BOLO(estados)){

                  if (MSG_TIPO(msg_recebida) == TAMANHO){
                     /* Verifica se é possível transferir o arquivo... */
                     struct statvfs buf;
                     statvfs("./objetos", &buf);

                     unsigned long tamanho_tesouro = *((unsigned long*) &msg_recebida[5]);
                     printf("        [B] Tamanho do Tesouro: %lu B\n", tamanho_tesouro);

                     erro_tamanho = (tamanho_tesouro > (buf.f_bavail * buf.f_bsize)) ? ESPACO_INSUFICIENTE : 0;

                     CL_PROCESSA_BOLO(estados)  = 0;
                     CL_PROCESSA_DADOS(estados) = 1;
                  }

                  else if (MSG_TIPO(msg_recebida) == ERRO){
                     printf("        [E] Erro. Tesouro Não Coletado.\n");
                     CL_PROCESSA_BOLO(estados) = 0;
                     CL_PROCESSA_FIM(estados)  = 1;
                  }
                  
                  else if (MSG_TIPO(msg_recebida) == FIM_ARQUIVO){
                     CL_PROCESSA_BOLO(estados) = 0;
                     CL_PROCESSA_FIM(estados)  = 1;
                  }

               }

               /* (MR03) Mensagem DADOS 
                  * Mensagem a Enviar: ACK */
               else if (CL_PROCESSA_DADOS(estados) && MSG_TIPO(msg_recebida) == DADOS){

                  /* Abre o arquivo a transferir... */
                  printf("        [N] Nome do Tesouro: %s\n", &msg_recebida[5]);

                  strcpy(caminho_aqv, "./tesouro/");
                  strcpy(&caminho_aqv[10], (char*) &msg_recebida[5]);

                  novo_arquivo = fopen(caminho_aqv, "wb");

                  CL_PROCESSA_DADOS(estados)   = 0;
                  CL_PROCESSA_ARQUIVO(estados) = 1;
               }

               /* (MR04) Mensagem ARQUIVO (Texto, Imagem e Vídeo) 
                  * Mensagem a Enviar: ACK */
               else if (CL_PROCESSA_ARQUIVO(estados)){

                  /* Verifica se a sequência está correta...
                     * e adiciona os dados no arquivo... */
                  if (MSG_TIPO(msg_recebida) == FIM_ARQUIVO){
                     CL_PROCESSA_ARQUIVO(estados) = 0;
                     CL_PROCESSA_FIM(estados)     = 1;
                  }

                  else if (MSG_SEQUENCIA(msg_recebida) == sequencia_atual){
                     fwrite((void*) &msg_recebida[5], MSG_TAMANHO(msg_recebida), 1, novo_arquivo);
                     sequencia_anterior = sequencia_atual;
                     sequencia_atual = (sequencia_atual + 1) % 32;
                  }

               }

               /* (MR05) Mensagem FIM_ARQUIVO 
                  * Mensagem a Enviar: MOVIMENTO */
               if (CL_PROCESSA_FIM(estados)){

                  /* Fecha o arquivo e exibe na tela... */
                  if (tem_tesouro){
                     fclose(novo_arquivo);

                     printf("        [Y] Tesouro Coletado com Sucesso!\n");

                     char* abrir_aqv = (char*) malloc(25 * sizeof(char));
                     strcpy(abrir_aqv, "xdg-open ");
                     strcpy(&abrir_aqv[9], caminho_aqv);
                     //system(abrir_aqv);
                     free(abrir_aqv);
                  }

                  break;

               }

            
               /* Parte II: Envio de Mensagens */
               /* Calcula a próxima mensagem a enviar, e a envia. */
               unsigned char msg_tipo_cl = MSG_TIPO(msg_recebida);

               if (msg_tipo_cl == OK){
                  if (d) printf("        [D] Envia ACK-OK.\n");
                  cria_mensagem(msg_enviar, 0, 0, ACK, NULL);
               }

               else if (msg_tipo_cl == TAMANHO){
                  if (erro_tamanho == 0){
                     if (d) printf("        [D] Envia ACK-TAMANHO.\n");
                     cria_mensagem(msg_enviar, 0, 0, ACK, NULL);
                  }
                  else if (erro_tamanho == ESPACO_INSUFICIENTE){
                     if (d) printf("        [D] Envia ERRO (Espaço).\n");
                     cria_mensagem(msg_enviar, 1, 0, ERRO, &erro_tamanho);
                     copia_mensagem(msg_enviar, msg_anterior);
                  }
               }

               else if (msg_tipo_cl == DADOS){
                  if (d) printf("        [D] Envia ACK-DADOS.\n");
                  cria_mensagem(msg_enviar, 0, 0, ACK, NULL);
               }

               else if (msg_tipo_cl == TEXTO){
                  if (d) printf("        [D] Envia ACK-ARQUIVO [%d].\n", sequencia_anterior);
                  cria_mensagem(msg_enviar, 0, sequencia_anterior, ACK, NULL);
               }

               else if (msg_tipo_cl == ERRO){
                  if (d) printf("        [D] Envia ACK-ERRO.\n");
                  cria_mensagem(msg_enviar, 0, 0, ACK, NULL);
               }

               //else if (msg_tipo_cl == NACK){
               //   if (d) printf("        [D] Envia MENSAGEM ANTERIOR (Nack).\n");
               //   if (MSG_TIPO(msg_anterior) != ACK) 
               //      copia_mensagem(msg_anterior, msg_enviar);
               //}
               
               envia_mensagem(msg_enviar, soquete);

               envia_ou_recebe = ESTADO_RECEBE;
               
            }
            
            /* (2) Cliente RECEBE uma Mensagem */
            else if (envia_ou_recebe == ESTADO_RECEBE){
               validade = recebe_mensagem(msg_recebida, soquete, tempo_timeout);

               /* (ST01) Timeout atingido... */
               if (validade == MSG_TIMEOUT){
                  num_timeouts++;
                  if (num_timeouts > 30){
                     fprintf(stderr, "Não foi possível continuar a conexão. Conexão finalizada...\n");
                     exit(-1);
                  }

                  tempo_timeout = tempo_timeout + 1000;

                  if (d) printf("        [D] TIMEOUT. Reenvia [0x%02x] e Espera [%ds]...\n", MSG_TIPO(msg_anterior), tempo_timeout / 1000);
                  if (MSG_TIPO(msg_enviar) != ACK && MSG_TIPO(msg_enviar) != NACK)
                     envia_mensagem(msg_anterior, soquete);
               }

               /* (ST02) Mensagem Recebida contém Erro... */
               else if (validade == MSG_ERRO_CHECK){
                  if (d) printf("        [D] Mensagem [0x%02x] com Erro...\n", MSG_TIPO(msg_recebida));
                  cria_mensagem(msg_enviar, 0, 0, NACK, NULL);
                  envia_mensagem(msg_enviar, soquete);

                  num_timeouts    = 0;
                  tempo_timeout   = TIMEOUT_INICIAL;
               }
               
               /* (ST03) Mensagem Recebida Válida! */
               else {
                  num_timeouts    = 0;
                  tempo_timeout   = TIMEOUT_INICIAL;
                  envia_ou_recebe = ESTADO_ENVIA;
               }
            }
         }

         /* Fim de jogo! Parabéns por completar o nosso jogo! */
         if (fim_de_jogo(MatrizTabuleiro, TAM_TABULEIRO)) break;
      }

      /* Jogou Terminou! */
      system("clear");
      printf("\n              [Fim de Jogo]            \n");
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
         
         /* Flags de Controle específicas do Servidor... */
         Tesouro_t tsr;
         unsigned char msg_tipo_sv;
         unsigned char num_tesouro;
         unsigned char bytes_lidos;
         unsigned char id_erro;

         unsigned long tamanho;

         /* Inicializa as Flags e Variáveis */
         envia_ou_recebe     = ESTADO_RECEBE;
         sequencia_atual     = 0;

         cria_mensagem(msg_enviar, 0, 0, LIVRE01, NULL);
         copia_mensagem(msg_enviar, msg_anterior);

         SV_PROCESSA_MOVIMENTO(estados) = 1;

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
   
               /* (MR01) Mensagem MOVIMENTO */
               if (SV_PROCESSA_MOVIMENTO(estados) && (MSG_TIPO(msg_recebida) == MV_BX || MSG_TIPO(msg_recebida) == MV_CM || 
                   MSG_TIPO(msg_recebida) == MV_EQ || MSG_TIPO(msg_recebida) == MV_DI)){
                  int coord_x_anterior = TAM_TABULEIRO - 1 - coord_jogador.l;
                  int coord_y_anterior = coord_jogador.c;

                  MatrizTabuleiro[coord_jogador.l][coord_jogador.c] = '*';
                  
                  /* Processa o Movimento */
                  switch( MSG_TIPO(msg_recebida) ){
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
                  
                  SV_PROCESSA_MOVIMENTO(estados) = 0;
                  SV_MONTA_TAMANHO_SV(estados)   = 1;
               }

               /* (MR02) Mensagem ACK */
               else if (MSG_TIPO(msg_recebida) == ACK){

                  /* (ACK01) Mandou TESOURO -> Envia TAMANHO ou FIM_RODADA */
                  if (SV_MONTA_TAMANHO_SV(estados) && MSG_TIPO(msg_anterior) == OK){

                     /* Tesouro encontrado -> Envia o Tamanho do Tesouro. */
                     if (num_tesouro != 0){       

                        tsr = Tesouros[num_tesouro - 1];

                        /* Não foi possível abrir o arquivo -> Erro. */
                        if (tsr.arq_tesouro == NULL){
                           id_erro = SEM_PERMISSAO_ACESSO;
                           printf("        [E] Não foi possível abrir o tesouro %d.\n", num_tesouro);

                           SV_MONTA_ERRO(estados) = 1;
                        }

                        /* Arquivo está aberto -> Envia o Tamanho. */
                        else {
                           id_erro = 0;

                           printf("        [N] Nome do Tesouro: %s\n", tsr.nome_tesouro);

                           struct stat buf_dados;
                           strcpy(caminho_aqv, "./objetos/");
                           strcpy(&caminho_aqv[10], tsr.nome_tesouro);

                           stat(caminho_aqv, &buf_dados);
                           tamanho = buf_dados.st_size;

                           printf("        [B] Tamanho do Tesouro: %lu B\n", tamanho);

                           SV_MONTA_DADOS(estados) = 1;
                        }
                     }

                     /* Tesouro NÃO encontrado -> Acaba Rodada. */
                     else
                        SV_PROCESSA_MOVIMENTO(estados) = 1;
                     
                     SV_MONTA_TAMANHO_SV(estados) = 0;
                  }

                  /* (ACK04) Mandou TEXTO, IMAGEM ou VIDEO -> Envia TEXTO, IMAGEM, VIDEO ou FIM_ARQUIVO */
                  else if ((SV_MONTA_DADOS(estados) || SV_MONTA_ARQUIVO(estados)) && 
                           (MSG_TIPO(msg_anterior) == DADOS || MSG_TIPO(msg_anterior) == TEXTO || 
                           MSG_TIPO(msg_anterior) == IMAGEM || MSG_TIPO(msg_anterior) == VIDEO)){

                     /* Atualiza o número da sequência do arquivo, para controle de fluxo. */
                     if (SV_MONTA_DADOS(estados) && MSG_TIPO(msg_anterior) == DADOS){
                        sequencia_atual    = 0;
                        sequencia_anterior = 0;
                        bytes_lidos = fread((void*) buffer, 1, 127, tsr.arq_tesouro);

                        if (bytes_lidos == 0)
                           SV_PROCESSA_MOVIMENTO(estados) = 1;
                        else 
                           SV_MONTA_ARQUIVO(estados)      = 1;
                        
                        SV_MONTA_DADOS(estados) = 0;
                     }

                     else if (SV_MONTA_ARQUIVO(estados)) {
                        if (MSG_SEQUENCIA(msg_recebida) == sequencia_atual){
                           sequencia_anterior = sequencia_atual;
                           sequencia_atual = (sequencia_atual + 1) % 32;
                           
                           bytes_lidos = fread((void*) buffer, 1, 127, tsr.arq_tesouro);
                           if (bytes_lidos == 0){
                              SV_MONTA_ARQUIVO(estados)      = 0;
                              SV_PROCESSA_MOVIMENTO(estados) = 1;
                           }
                        }
                     }
                  }

                  /* (ACK05) Mandou ERRO -> Envia FIM_ARQUIVO */
                  else if (SV_MONTA_ERRO(estados) && MSG_TIPO(msg_anterior) == ERRO){
                     SV_MONTA_ERRO(estados)         = 0;
                     SV_PROCESSA_MOVIMENTO(estados) = 1;
                  }
               }

               /* (MR04) Mensagem ERRO */
               else if (MSG_TIPO(msg_recebida) == ERRO){

                  printf("        [E] Espaço Insuficiente para o tesouro.\n");

                  SV_PROCESSA_MOVIMENTO(estados) = 1;

               }

               /* Parte II: Envio de Mensagens */
               /* Calcula a próxima mensagem a enviar, e a envia. */

               msg_tipo_sv = MSG_TIPO(msg_recebida);

               if (msg_tipo_sv == MV_BX || msg_tipo_sv == MV_CM || 
                   msg_tipo_sv == MV_EQ || msg_tipo_sv == MV_DI)
                  cria_mensagem(msg_enviar, 1, 0, OK, &num_tesouro);
               
               else if (msg_tipo_sv == ACK){
                  unsigned char tipo_anterior = MSG_TIPO(msg_anterior);
        
                  if (tipo_anterior == OK){
                     if (num_tesouro == 0)
                        cria_mensagem(msg_enviar, 0, 0, FIM_ARQUIVO, NULL);
                     else {
                        if (id_erro == ESPACO_INSUFICIENTE)
                           cria_mensagem(msg_enviar, 1, 0, ERRO, &id_erro);
                        else if (id_erro == 0)
                           cria_mensagem(msg_enviar, sizeof(unsigned long), 0, TAMANHO, (unsigned char*) &tamanho);
                     }
                  }

                  else if (tipo_anterior == TAMANHO)
                     cria_mensagem(msg_enviar, 6, 0, DADOS, (unsigned char*) tsr.nome_tesouro);
                  
                  else if (tipo_anterior == DADOS || tipo_anterior == TEXTO){
                     if (tipo_anterior == TEXTO){
                        if (MSG_SEQUENCIA(msg_recebida) == sequencia_anterior){
                           if (bytes_lidos > 0) cria_mensagem(msg_enviar, bytes_lidos, sequencia_atual, TEXTO, buffer);
                           else cria_mensagem(msg_enviar, 0, 0, FIM_ARQUIVO, NULL);
                        } 

                        else {
                           copia_mensagem(msg_anterior, msg_enviar);
                        }
                     }
                     
                     else if (tipo_anterior == DADOS){
                        if (bytes_lidos > 0) cria_mensagem(msg_enviar, bytes_lidos, sequencia_atual, TEXTO, buffer);
                        else cria_mensagem(msg_enviar, 0, 0, FIM_ARQUIVO, NULL);
                     }
                  }

                  else if (tipo_anterior == ERRO)
                     cria_mensagem(msg_enviar, 0, 0, FIM_ARQUIVO, NULL);
               }

               else if (msg_tipo_sv == ERRO)
                  cria_mensagem(msg_enviar, 0, 0, FIM_ARQUIVO, NULL);
               
               else if (msg_tipo_sv == NACK){
                  printf("EU CAI NO NACK");
                  if (MSG_TIPO(msg_anterior) == OK){
                     cria_mensagem(msg_enviar, 1, 0, OK, &num_tesouro);
                     printf("EU CAI NO NACK OK");
                  }
                  else if (MSG_TIPO(msg_anterior) == TAMANHO){
                     cria_mensagem(msg_enviar, sizeof(unsigned long), 0, TAMANHO, (unsigned char*) &tamanho);
                     printf("EU CAI NO NACK TAMANHO");
                  }
                  else if (MSG_TIPO(msg_anterior) == DADOS){
                     cria_mensagem(msg_enviar, 6, 0, DADOS, (unsigned char*) tsr.nome_tesouro);
                     printf("EU CAI NO NACK DADOS");
                  }

                  else if (MSG_TIPO(msg_anterior) == TEXTO){
                     cria_mensagem(msg_enviar, bytes_lidos, sequencia_anterior, TEXTO, buffer);
                     printf("EU CAI NO NACK TEXTO");
                  }

                  else if (MSG_TIPO(msg_anterior) == FIM_ARQUIVO)
                     cria_mensagem(msg_enviar, 0, 0, FIM_ARQUIVO, NULL);
                     printf("EU CAI NO NACK FIM_ARQUIVO");
               }

               copia_mensagem(msg_enviar, msg_anterior);
               envia_mensagem(msg_enviar, soquete);

               envia_ou_recebe = ESTADO_RECEBE;
            }

            /* (2) Servidor RECEBE uma Mensagem */
            else if (envia_ou_recebe == ESTADO_RECEBE){
               validade = recebe_mensagem(msg_recebida, soquete, tempo_timeout);

               /* (ST01) Timeout atingido... */
               if (validade == MSG_TIMEOUT){
                  num_timeouts++;
                  if (num_timeouts > 30){
                     fprintf(stderr, "Não foi possível continuar a conexão. Conexão finalizada...\n");
                     exit(-1);
                  }

                  tempo_timeout = tempo_timeout + 1000;
                  
                  if (d) printf("        [D] TIMEOUT. Reenvia e Espera [%ds]...\n", tempo_timeout / 1000);
                  if (MSG_TIPO(msg_enviar) != ACK && MSG_TIPO(msg_enviar) != NACK)
                     envia_mensagem(msg_anterior, soquete);
               }

               /* (ST02) Mensagem Recebida contém Erro... */
               //else if (validade == MSG_ERRO_CHECK){
                  //if (d) printf("        [D] Mensagem com Erro...\n");
                  //cria_mensagem(msg_enviar, 0, 0, NACK, NULL);
                  //envia_mensagem(msg_enviar, soquete);
               //}
               
               /* (ST03) Mensagem Recebida Válida! */
               else {
                  num_timeouts    = 0;
                  tempo_timeout   = TIMEOUT_INICIAL;
                  envia_ou_recebe = ESTADO_ENVIA;
               }
            }
         }

         sleep(1);
         system("clear");
      }
   }

   /* Libera a Memória das Estruturas */
   liberaMatriz(MatrizTabuleiro, TAM_TABULEIRO);
   free(msg_enviar);
   free(msg_anterior);
   free(msg_recebida);
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */