#include "jogo.h"

void inicia_jogo_tesouro(int soquete, Usuario usuario)
{
   /* Variáveis do Jogo */


   /* <Código do Cliente>
    * (1) A matriz do Tabuleiro contém apenas as posições já visitadas e também
    * mantém a posição do Jogador.
    * (2) O Cliente deve RECEBER mensagens do tipo ARQUIVO (Texto, Imagem
    * e Vídeo), não enviá-las.
    * (3) ...*/
   if (usuario == CLIENTE){

   }
   /* <Código do Servidor>
    * (1) A matriz do Tabuleiro contém apenas as localizações dos tesouros.
    * (2) O Cliente deve ENVIAR mensagens do tipo ARQUIVO (Texto, Imagem
    * e Vídeo), não recebê-las.
    * (3) ...*/
   else if (usuario == SERVIDOR){

   }

   else {
      return;
   }
}