#include <unistd.h>
#include <stdlib.h>
int main(int argc, char const *argv[]) {
   char buff[300];
   read(0, buff, 300);
   write(1, buff, 300);
   return 0;
}
