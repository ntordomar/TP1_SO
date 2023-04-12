#include <unistd.h>
#include <stdlib.h>
int main(int argc, char const *argv[]) {
   while(1) {
      char buff[300] = {0};
      int n = read(0, buff, 300);
      buff[n]=0;
      write(1, buff, 300);
   }
   return 0;

}
