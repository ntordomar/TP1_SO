

#define BUFFER 1024
#define MD5_LEN 33
#define CHILD 0
#define READ 0
#define WRITE 1
#define MAX_CANT_OF_WORKERS 5


typedef struct Response {
    char md5[33];
    int pid;
    char name[100];
} Response;

void normalize_string(char* str, char* ans);
void print_process_information(struct Response response);
int get_max_from_array(int * array, int num_fd);
char is_file(char * path);
void error_call(char * message_error, int return_number);
void manage_worker_pipes(int * pipe_files, int * pipe_data);
void sending_first_files(int * file_to_send, int first_amount, int * workers_fds_write, char ** files_paths, char * pending_jobs, int num_workers);
