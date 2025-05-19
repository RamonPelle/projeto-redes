#ifndef __MENSAGEM_H__
#define __MENSAGEM_H__

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#define MSG_INICIO(msg)    msg[0]
#define MSG_TAMANHO(msg)   msg[1]
#define MSG_SEQUENCIA(msg) msg[2]
#define MSG_TIPO(msg)      msg[3]
#define MSG_CHECKSUM(msg)  msg[4]
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
typedef char* mensagem_t;

#define ACK           0x00     /* Mensagem ACK */
#define NACK          0x01     /* Mensagem NACK */
#define OK            0x02     /* Mensagem OK + ACK */
#define LIVRE_01      0x03     /* Mensagem Livre 01 */
#define TAMANHO       0x04     /* Mensagem TAMANHO */
#define DADOS         0x05     /* Mensagem TAMANHO */
#define TEXTO         0x06     /* Mensagem TEXTO + ACK */
#define VIDEO         0x07     /* Mensagem VIDEO + ACK */
#define IMAGEM        0x08     /* Mensagem IMAGEM + ACK */
#define FIM_ARQUIVO   0x09     /* Mensagem FIM DO ARQUIVO */
#define MOVE_DIREITA  0x0A     /* Mensagem MOVIMENTO DIREITA */
#define MOVE_CIMA     0x0B     /* Mensagem MOVIMENTO CIMA */
#define MOVE_BAIXO    0x0C     /* Mensagem MOVIMENTO BAIXO */
#define MOVE_ESQUERDA 0x0D     /* Mensagem MOVIMENTO ESQUERDA */
#define LIVRE_02      0x0E     /* Mensagem Livre 02 */
#define ERRO          0x0F     /* Mensagem ERRO*/

#define SEM_PERMISSAO_ACESSO 0 /* ERR01: Não foi possível abrir o arquivo. */
#define ESPACO_INSUFICIENTE  1 /* ERRO2: Não há espaço suficiente para o arquivo. */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
int cria_mensagem(mensagem_t msg, unsigned char tam_dados, unsigned char seq, 
                                  unsigned char tipo, unsigned char* dados);
                  
int envia_mensagem  (mensagem_t msg, int socket);
int recebe_mensagem (mensagem_t msg, int soquete, int timeoutMillis);
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#endif