#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "information.h"
#include "worker.h"
#include <stdio.h>
#include <sys/stat.h>


int main(int argc, char const *argv[]) {
   while(1) {

      char * buff;
      
      buff = fgets(buff, 10, 0);//esta leyendo los dos archivos
      // aca haces primero un write para el primero y despues otro para el segundo? 
      
      //printf("%s\n",buff);


      // struct Response response;

      // char md5[MD5_LEN];
      // md5_calc(buff, md5);

      // strcpy(response.md5, md5);
      // strcpy(response.name, buff);
      // response.pid = getpid();

      // write(1, &response, sizeof(response));
   }
   return 0;
}

void md5_calc(char * file, char * answer) {
   char command[BUFFER] = {0};
   FILE *fp; // the file we are going to send to md5

   snprintf(command, BUFFER, "md5sum %s", file); // creates the command we will execute
   
   fp = popen(command, "r"); // creates a pipe linked with a terminal and sends the command

   // Error case
   if (fp == NULL){
      perror("md5 Failed");
      exit(1);
   }

   if (fgets(answer, MD5_LEN, fp) != NULL) {
        // Remove the trailing newline character
        answer[strcspn(answer, "\n")] = '\0'; // changes the '\n' for '\0' -> end of string
   }

   pclose(fp);
}
