#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "information.h"
#include "worker.h"
#include <stdio.h>
#include <sys/stat.h>


int main(int argc, char const *argv[]) {
   while(1) {

      char buff[300] = {0};
      int n = read(0, buff, 300); //esta leyendo los dos archivos
      // aca haces primero un write para el primero y despues otro para el segundo? 
      buff[n]=0;


      struct Response response;

      char md5[17];
      md5_calc(buff, md5);

      strcpy(response.md5, md5);
      strcpy(response.name, buff);
      response.pid = getpid();

      write(1, &response, sizeof(response));
   }
   return 0;
}

void md5_calc(char * file, char * answer) {
   char command[BUFFER] = {0};
   FILE *fp;

   snprintf(command, BUFFER, "md5sum %s", file);
   
   fp = popen(command, "r");

   if (fp == NULL){
      perror("md5 Failed");
      return;
   }

   if (fgets(answer, 33, fp) != NULL) {
        // Remove the trailing newline character
        answer[strcspn(answer, "\n")] = '\0';
   }

   pclose(fp);
}
