#ifndef __MENSAGEM_H__
#define __MENSAGEM_H__

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
typedef struct mensagem_s {
   unsigned char marcador_inicio;
   unsigned char tamanho_dados;
   unsigned char numero_sequencia;
   unsigned char tipo_mensagem;
   unsigned char checksum;
   void* dados;
} mensagem_t;

typedef enum {
   ACK           = 0,
   NACK          = 1,
   OK_ACK        = 2,
   TAMANHO       = 4,
   DADOS         = 5,
   TEXTO         = 6,
   VIDEO         = 7,
   IMAGEM        = 8,
   FIM_ARQUIVO   = 9,
   MOVE_DIREITA  = 10,
   MOVE_CIMA     = 11,
   MOVE_BAIXO    = 12,
   MOVE_ESQUERDA = 13,
   ERRO          = 15
} TipoMensagem_t;

typedef enum {
   SEM_PERMISSAO_ACESSO = 0,
   ESPACO_INSUFICIENTE  = 1
} CodigoErro_t;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
mensagem_t cria_mensagem (unsigned char tam_dados, unsigned char seq, 
                          unsigned char tipo, void* dados);
                  
int envia_mensagem  (mensagem_t msg, int socket);
int recebe_mensagem (mensagem_t* msg, int socket);
int trata_mensagem  (mensagem_t msg);

#endif