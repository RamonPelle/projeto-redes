#ifndef __MENSAGEM_H__
#define __MENSAGEM_H__

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Macros p/ obter as informações de protocolo da mensagem */
#define MSG_INICIO(msg)    msg[0]
#define MSG_TAMANHO(msg)   msg[1]
#define MSG_SEQUENCIA(msg) msg[2]
#define MSG_TIPO(msg)      msg[3]
#define MSG_CHECKSUM(msg)  msg[4]
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Definições p/ maior clareza do programa */
/* Define a mensagem sendo um vetor de char. */
typedef unsigned char* mensagem_t;

#define ACK           0x00     /* Mensagem ACK                               */
#define NACK          0x01     /* Mensagem NACK                              */
#define OK            0x02     /* Mensagem OK + ACK                          */
#define TESOURO       0x03     /* Mensagem TESOURO                           */
#define TAMANHO       0x04     /* Mensagem TAMANHO                           */
#define DADOS         0x05     /* Mensagem DADOS                             */
#define TEXTO         0x06     /* Mensagem TEXTO + ACK                       */
#define VIDEO         0x07     /* Mensagem VIDEO + ACK                       */
#define IMAGEM        0x08     /* Mensagem IMAGEM + ACK                      */
#define FIM_ARQUIVO   0x09     /* Mensagem FIM DO ARQUIVO                    */
#define MV_DI         0x0A     /* Mensagem MOVIMENTO DIREITA                 */
#define MV_CM         0x0B     /* Mensagem MOVIMENTO CIMA                    */
#define MV_BX         0x0C     /* Mensagem MOVIMENTO BAIXO                   */
#define MV_EQ         0x0D     /* Mensagem MOVIMENTO ESQUERDA                */
#define FIM_RODADA    0x0E     /* Mensagem FIM_RODADA                        */
#define ERRO          0x0F     /* Mensagem ERRO                              */

#define SEM_PERMISSAO_ACESSO 0 /* ERR01: Não foi possível abrir o arquivo.   */
#define ESPACO_INSUFICIENTE  1 /* ERRO2: Espaço suficiente para o arquivo.   */

#define MSG_VALIDA      1      /* Mensagem Válida.                           */
#define MSG_INVALIDA    0      /* Mensagem Inválida.                         */
#define MSG_TIMEOUT    -1      /* Timeout: Nenhuma resposta recebida.        */
#define MSG_ERRO_CHECK -2      /* Mensagem Inválida: Checksum incorreto.     */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Funções de Transmissão de Mensagens */
int cria_mensagem(mensagem_t msg, unsigned char tam_dados, unsigned char seq, 
                                  unsigned char tipo, unsigned char* dados);
                  
int envia_mensagem  (mensagem_t msg, int socket);
int recebe_mensagem (mensagem_t msg, int soquete, int timeoutMillis);
int copia_mensagem  (mensagem_t msg_original, mensagem_t msg_copia);
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#endif