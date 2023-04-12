#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "information.h"
int main(int argc, char const *argv[]) {
   while(1) {

      char buff[300] = {0};
      int n = read(0, buff, 300); //esta leyendo los dos archivos
      // aca haces primero un write para el primero y despues otro para el segundo? 
      buff[n]=0;
      
      struct Response response;
      strcpy(response.md5, "12345");
      strcpy(response.name, buff);
      response.pid = getpid();

      write(1, &response, sizeof(response));
   }
   return 0;
}
