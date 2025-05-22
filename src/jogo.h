#ifndef __JOGO_H_
#define __JOGO_H_

#include "defines.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#define ESTADO_ENVIA  1
#define ESTADO_RECEBE 2
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
typedef struct PosicaoJogador_s{
   int l, c;
} PosicaoJogador_t;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void inicia_jogo_tesouro(int soquete, Usuario usuario);
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#endif