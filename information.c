// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "information.h"

char is_file(char * path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

int get_max_from_array(int * array, int num_fd){
    int max = array[0];
    int i;
    for(i = 1; i < num_fd; i++) {
        if(array[i] > max) {
            max = array[i];
        }
    }
    return max;
}

void error_call(char * message_error, int return_number) {
    perror(message_error);
    exit(return_number);
}

void normalize_string(char* str, char* ans) {
    int i;
    int j =0;
    for (i = 0; i<strlen(str); i++){
        if (str[i] == ' '){
            ans[j++] = '\\';
        }
        ans[j++] = str[i];
    }
    ans[j] = '\n';
}

void print_process_information(struct Response response){
    printf("-------------------\n");
    printf("PID: %d\n",response.pid);
    printf("name: %s",response.name);
    printf("md5: %s\n",response.md5);
    printf("-------------------\n");
}

void manage_worker_pipes(int * pipe_files, int * pipe_data){
    close(0); // From now on, recieving information from the pipe instead od stdin
    dup(pipe_files[READ]);
    close(1); // form now on, sending information to the pipe instead of stdout
    dup(pipe_data[WRITE]);
    close(pipe_files[READ]);
    close(pipe_data[WRITE]);
    close(pipe_data[READ]);
    close(pipe_files[WRITE]);
}

void sending_first_files(int * file_to_send, int first_amount, int * workers_fds_write, char ** files_paths, char * pending_jobs, int num_workers){
    int fa;
    int work;
    int aux_jobs;
    for (work = 0; work < num_workers; work++){ // iterate in workers
        for(fa = 0; fa < first_amount; fa++) { // iterate in first ammount of files
            if(write(workers_fds_write[work], files_paths[*file_to_send], strlen(files_paths[*file_to_send])) == -1) {
                error_call("Write failed", 1);
            }
            (*file_to_send)++;
            aux_jobs = pending_jobs[work];
            pending_jobs[work] = aux_jobs + 1;
        }
    }
}

void close_shared_memory(void * ptr, size_t length, char* name, int fd) {
    close(fd);
    if(munmap(ptr, length) == -1) {
        error_call("Error on unmaping the shared memory", 1);
    }
    if(shm_unlink(name) == -1) {
        error_call("Error on unlinking the shared memory", 1);
    }
}