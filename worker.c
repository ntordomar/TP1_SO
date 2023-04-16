// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include "lib.h"
#include "worker.h"



int main(int argc, char const *argv[]) {
   // creates the response it will send to aplication.
   struct Response response;
   // It will never stop. Application process will kill each of them.
   while(1) {
      char *line = NULL;
      size_t len = 0;
      //recieve the path
      getline(&line, &len, stdin);
      char md5[MD5_LEN];
      md5_calc(line, md5);

      strcpy(response.md5, md5);
      strcpy(response.name, line);
      response.pid = getpid();
      // write the response to application.
      write(1, &response, sizeof(response));
      free(line);
   }
   
   return 0;
}

void md5_calc(char * file, char * answer) {
   char command[BUFFER] = {0};
   FILE *fp; // the file we are going to send to md5

   // creates the command we will execute
   snprintf(command, BUFFER, "md5sum %s", file);
   
   // creates a pipe linked with a terminal and sends the command
   fp = popen(command, "r"); 

   // Error case
   if (fp == NULL){
      perror("md5 Failed");
      exit(1);
   }

   if (fgets(answer, MD5_LEN, fp) != NULL) {
      // changes the '\n' for '\0' -> end of string
      answer[strcspn(answer, "\n")] = '\0'; 
   }

   pclose(fp);
}
