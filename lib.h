#include <dirent.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/wait.h>


#include "error.h"


#define BUFFER 1024
#define MD5_LEN 33
#define CHILD 0
#define READ 0
#define WRITE 1
#define MAX_CANT_OF_WORKERS 5
#define MAX_FILES 500
#define SHM_SIZE (MAX_FILES * sizeof(Response))


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

int filter_normalize_files(int num_files, char ** argv, char ** files_paths);

FILE * create_file(char * name, char * mode);


/* --- PIPES AND PROCESSES --- */
void create_pipe(int * pipe_fd);
void close_pipe(int * pipe, int fd);
int create_process();
void start_process(char * path_name, char * parameters[]);
void select_process(int max_fd, fd_set * read_fds);
void read_process(int fd, Response * response);
void write_process(int fd, char * file_path);

/* --- SHARED MEMORY --- */
Response * create_shared_memory(char * name, int * shm_fd);
Response * open_shared_memory(char * name, int * shm_fd);
void unmap_shared_memory(Response * pointer_to_shm, int * shm_fd);
void unlink_shared_memory(char * name);

/* --- SEMAPHORE --- */
sem_t * create_semaphore(char * name, int value);
sem_t * open_semaphore(char * name, int value);
void close_semaphore(sem_t * semaphore);
void unlink_semaphore(char * name);

void unlink_exec_failed(char * sem1, char * sem2, char * shm);
