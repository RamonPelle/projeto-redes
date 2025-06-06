#ifndef __JOGO_H_
#define __JOGO_H_

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#define ESTADO_ENVIA  1
#define ESTADO_RECEBE 2
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
typedef enum {
   CLIENTE  = 0,
   SERVIDOR = 1
} Usuario;

typedef struct PosicaoJogador_s{
   int l, c;
} PosicaoJogador_t;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void jogo_tesouro(int soquete, Usuario usuario);
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#endif