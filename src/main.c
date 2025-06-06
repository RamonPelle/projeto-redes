#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "socket.h"
#include "jogo.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
int main(int argc, char* argv[]) {
   int soquete;
   
   /* Primeiro Passo: Vamos criar um Soquete da Interface de Rede! */
   if (argv[2]) soquete = cria_socket(argv[2]);
   else { 
      fprintf(stderr, "Uso: %s (-c|-s) <interface-rede>\n", argv[0]); 
      exit(-1);
   }

   int opt;
   /* Segundo Passo: Executar sendo CLIENTE ou SERVIDOR! */
   while ((opt = getopt(argc, argv, "cs")) != -1)
      switch (opt) {
         case 'c':
            jogo_tesouro(soquete, CLIENTE);
            break;
         case 's':
            jogo_tesouro(soquete, SERVIDOR);
            break;
         default:
            fprintf(stderr, "Uso: %s (-c|-s) <interface-rede>\n", argv[0]);
            break;
      }

   return 0;
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
