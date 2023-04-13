// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "information.h"
#include "worker.h"
#include <stdio.h>
#include <sys/stat.h>


int main(int argc, char const *argv[]) {
   struct Response response;
   while(1) {
      char *line = NULL;
      size_t len = 0;
     getline(&line, &len, stdin);
      // n = read(0, buff, 14); //esta leyendo los dos archivos
      // // aca haces primero un write para el primero y despues otro para el segundo? 
      // buff[n] = 0;

      char md5[MD5_LEN];
      md5_calc(line, md5);

      strcpy(response.md5, md5);
      strcpy(response.name, line);
      response.pid = getpid();
      write(1, &response, sizeof(response));
      free(line);
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
