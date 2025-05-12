/* Bibliotecas Padrão */
#include <sys/socket.h>

/* Bibliotecas Locais */
#include "mensagem.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
mensagem_t cria_mensagem(unsigned char tam_dados, unsigned char seq, 
                         unsigned char tipo, void* dados)
{
   mensagem_t msg;

   /* Marcador de Início = 01111110 */
   msg.marcador_inicio  = 126;
   msg.tamanho_dados    = tam_dados;            
   msg.numero_sequencia = seq;
   msg.tipo_mensagem    = tipo;
   msg.dados            = dados;
   //msg.checksum         = calcula_checksum();

   return msg;
}

int envia_mensagem(mensagem_t msg, int socket)
{
   //send(socket, (void*) &msg, 5 + msg.tamanho_dados, 0);
}

int recebe_mensagem(mensagem_t* msg, int socket){
   //void* buffer;
   //mensagem_t temp_msg;

   //recv(socket, buffer, 132, 0);
   //temp_msg = (mensagem_t) buffer;
   
   //msg->
}

int trata_mensagem(mensagem_t msg){

}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */