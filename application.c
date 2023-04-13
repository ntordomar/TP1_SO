// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/select.h>
#include "information.h"



int main(int argc, char *argv[]) {
    int aux = 0;
    int num_files = argc; 
    char * files_paths[num_files];

    int i;
    int real_file_count = 0;
    for(i = 1; i < num_files; i++) {
        if(is_file(argv[i])) {
            char * buff = malloc(strlen(argv[i]) * 2); // We decided to duplicate the length of the string in case the file has a lot of spaces.
            if(buff == NULL){
                error_call("Could not allocate the necesary memory\n",1);
            }
            normalize_string(argv[i], buff);
            files_paths[real_file_count++] = buff;
            printf("%s\n",files_paths[i-1]);
        }
    }
    
    if(real_file_count == 0){
        printf("no files recieved\n");
        return 0;
    }
    
    // from this point, we have in the files_paths array all files.
    int num_workers = real_file_count < MAX_CANT_OF_WORKERS ? real_file_count : MAX_CANT_OF_WORKERS;
    char * worker_parameters[] = { "./worker", NULL };  // parameters for execve
    int workers_fds[2][num_workers];                    //matrix of workers file descriptors
    
    int first_amount = (int) (0.2*real_file_count/num_workers);
    if(first_amount == 0) {
        first_amount = 1;
    } 
    // in the following for we will create all workers and their pipes.
    for(i = 0; i < num_workers; i++) { 
        //Pipe that sends files to worker
        int pipe_files[2];
        //Pipe that recieves data from worker
        int pipe_data[2];

        //Creating pipes
        if(pipe(pipe_files) == -1 || pipe(pipe_data) == -1) {
            error_call("pipe call failed", 1);
        }
          //Saving file descriptors
        workers_fds[READ][i] = pipe_data[READ];
        workers_fds[WRITE][i] = pipe_files[WRITE];

        //Creating worker
        int worker_fork = fork();
        
        if(worker_fork == -1) {
            error_call("pipe call failed", 1);
        }
        
        //Closing FD from child and changing stdin and stdout
        if(worker_fork == CHILD) {
            int j;
            for(j=0; j<i;j++){
                close(workers_fds[READ][j]);
                close(workers_fds[WRITE][j]);
            }
            manage_worker_pipes(pipe_files,pipe_data); //Opening and closing desired fd before calling execve.
            execve("./worker", worker_parameters, 0);
        }
        //Closing pipes ends that the proccess is not going to use.
        close(pipe_files[READ]); 
        close(pipe_data[WRITE]);
    }


    char pending_jobs[num_workers]; // in this array we will save the amount of files each worker is processing
    for (i = 0; i < num_workers; i++) {
        pending_jobs[i] = 0;
    }

    int file_to_send = 0; // the number of the next file to send

    //Here we send 'first_ammount' of files to each worker.
    sending_first_files(&file_to_send, first_amount, workers_fds[WRITE],files_paths, pending_jobs, num_workers);

    // Now we will use select to check which workers sent the information.    
    fd_set read_fds; // set with the read file descriptors
    int max_fd = get_max_from_array(workers_fds[READ],num_workers);
    int ready_fds; // know how many workers are ready to read.
    Response response; // a struct with the information of the worker answer.
    int amount_read = 1;
    int aux_jobs;
    while(amount_read <= real_file_count) {
        FD_ZERO(&read_fds); // restore values of fds that are ready to read.
        for(i = 0; i<num_workers; i++) {
            FD_SET(workers_fds[READ][i], &read_fds);
        }
        
        ready_fds = select(max_fd + 1, &read_fds, NULL, NULL, NULL); //The function returns how many workers are ready to work
        
        if(ready_fds == -1) {
            error_call("Select failed.", 1);
        }
        // we iterate in each worker to see if its ready to read.
        for(i = 0; i < num_workers; i++) {
            if(FD_ISSET(workers_fds[READ][i], &read_fds)) { //FD_ISSET returns whether if a worker is blocked for reading.
                
                if(read(workers_fds[READ][i], &response, sizeof(response)) == -1) {
                    error_call("Error on read", 1);
                } else {
                    amount_read ++;
                    aux_jobs = pending_jobs[i];
                    pending_jobs[i] = aux_jobs -1;
                    printf("este es el numero %d de %d\n",aux, real_file_count);
                    aux++;
                    print_process_information(response);

                }
                if(!pending_jobs[i]){ //We keep track if a worker is doing any jobs before sending a new one
                    if(file_to_send < real_file_count){ 
                        if (write(workers_fds[WRITE][i], files_paths[file_to_send], strlen(files_paths[file_to_send])) == -1){
                            error_call("Write failed", 1);
                        }
                        file_to_send++;
                        aux_jobs = pending_jobs[i];
                        pending_jobs[i] = aux_jobs + 1;
                    }
                }
            }
        }
    }
    for(i = 0; i<real_file_count;i++){
        free(files_paths[i]);
    }
    printf("total files %d \n",real_file_count);
    return 0;
}

