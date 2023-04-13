#define BUFFER 1024
#define MD5_LEN 33
typedef struct Response {
    char md5[33];
    int pid;
    char name[100];
} Response;


