/* Bibliotecas Padrão */
#include <stdlib.h>
#include <time.h>

/* Bibliotecas Locais */
#include "mensagem.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/*                                 ✉ Criação                                 */
/*                                de Mensagens                               */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* [Função] calcula_checksum(): Calcula o Checksum de uma mensagem fornecida,
 * utilizando os campos de Início, Tamanho, Sequência, Tipo e Dados. */ 
unsigned char calcula_checksum(mensagem_t msg)
{
   unsigned char checksum = 0;
   
   if (msg == NULL) return 0;

   /* Checksum <- Tamanho + Sequência + Tipo */
   for (int i = 1; i < 4; i++)
      checksum += msg[i];
   /* Checksum <- Dados */
   for (int i = 5; i < 5 + MSG_TAMANHO(msg); i++)
      checksum += msg[i];
   
   return checksum;
}

/* [Função] cria_mensagem(): Verifica se uma mensagem fornecida é válida ou
 * não, baseado no protocolo de mensagens utilizado. */
int cria_mensagem(mensagem_t msg, unsigned char tam_dados, unsigned char seq, 
                  unsigned char tipo, unsigned char* dados)
{
   if (msg == NULL) return -1;

   MSG_INICIO(msg)    = 0x7E;
   MSG_TAMANHO(msg)   = tam_dados;            
   MSG_SEQUENCIA(msg) = seq;
   MSG_TIPO(msg)      = tipo;

   for (int i = 5; i < 5 + tam_dados; i++)
      msg[i] = dados[i - 5];

   MSG_CHECKSUM(msg) = calcula_checksum(msg);

   return 0;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/*                           ✉ Envio e Recebimento                           */
/*                                de Mensagens                               */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */                                  
/* [Função] mensagem_valida(): Verifica se uma mensagem fornecida é válida ou
 * não, baseado no protocolo de mensagens utilizado. */
int mensagem_valida(mensagem_t msg)
{
   /* (1) Verifica a INTEGRIDADE da Mensagem:
    * - Marcador de Início está errado? ZERO.
    * - Tamanho da Mensagem maior que o suportado? ZERO.
    * - Tipo da Mensagem diferente dos suportados? ZERO.
    * - Sequência de Mensagem diferente do suportado? ZERO. */
   if (MSG_INICIO(msg) != 0x7E) return MSG_INVALIDA;
   if (MSG_TAMANHO(msg) > 0X7F) return MSG_INVALIDA;
   if (MSG_TIPO(msg) > 0x0F)    return MSG_INVALIDA;
   if (MSG_SEQUENCIA(msg) > 31) return MSG_INVALIDA;

   /* (2) Verifica o CHECKSUM da Mensagem:
    * - Calcula o Checksum da Mensagem recebida 
    * - Checksum calculado é diferente do recebido? ZERO. */
   unsigned char chcksm;
   chcksm = calcula_checksum(msg);
   if (chcksm != MSG_CHECKSUM(msg)) return MSG_ERRO_CHECK;

   return MSG_VALIDA;
}

/* [Função] envia_mensagem(): Envia uma mensagem criada através de um soquete
 * de uma interface de rede. Caso a mensagem não seja enviada adequadamente,
 * retorna um valor de erro. */
int envia_mensagem(mensagem_t msg, int soquete)
{
   int bytes_enviados;

   if (protocolo_valido(msg)){
      bytes_enviados = send(soquete, msg, 5 + MSG_TAMANHO(msg), 0);
      return bytes_enviados;
   } else {
      return -1;
   }
}

long long timestamp(){
   struct timeval tp;
   gettimeofday(&tp, NULL);
   return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

/* [Função] recebe_mensagem(): Captura uma mensagem vinda de um soquete de uma
 * interface de rede. Caso uma mensagem não seja recebida em um certo tempo de
 * Timeout, retorna um valor de TIMEOUT. */
int recebe_mensagem(mensagem_t msg, int soquete, int timeoutMillis){
   /* Variáveis de Marcação de Tempo */
   long long comeco = timestamp();
   struct timeval timeout = {
      .tv_sec  = timeoutMillis / 1000,
      .tv_usec = (timeoutMillis % 1000) * 1000
   };

   /* Configuração do Soquete para o Uso do Timeout */
   setsockopt(soquete, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));

   /* Recebimento de Mensagem com Timeout */
   int bytes_lidos;
   do {
      bytes_lidos = recv(soquete, msg, 132, 0);
      if (mensagem_valida(msg) == MSG_VALIDA)
         return bytes_lidos;
      else if (mensagem_valida(msg) == MSG_ERRO_CHECK)
         return MSG_ERRO_CHECK;
   } while (timestamp() - comeco <= timeoutMillis);

   return MSG_TIMEOUT;
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */