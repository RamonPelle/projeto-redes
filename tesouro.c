#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

int main(int argc, char* argv[])
{
   setlocale(LC_ALL, "");
   
   FILE* tabuleiro = fopen("./tabuleiro.txt", "r");

   if (tabuleiro == NULL){
      fprintf(stderr, "Erro ao abrir o arquivo de código.\n");
      exit(1);
   }

   char buffer[256];
   char* str;

   do {
      str = fgets(buffer, 256, tabuleiro);
      if (str != NULL) printf("%s", buffer); 
   } while (str != NULL);

   fclose(tabuleiro);
}