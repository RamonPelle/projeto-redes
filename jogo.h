#ifndef __JOGO_H_
#define __JOGO_H_

#include "defines.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
typedef struct PosicaoJogador_s{
   int l, c;
} PosicaoJogador_t;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void inicia_jogo_tesouro(int soquete, Usuario usuario);
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#endif