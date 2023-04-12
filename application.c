#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "application.h"
#include <sys/stat.h>
#include <string.h>

//PREGUNTAS
// SI TIENE QUE SER VARIABLE LA CANTIDAD DE SLAVES
// TIENE QUE SER VARIABLE LA CANTIDAD DE PROCESOS QUE SE LE DA INCIIALMENTE A CADA WORKER.


int main(int argc, char *argv[]) {
    
    int num_files = argc; 
    char* files_paths[num_files];
    
    int i;
    int real_file_count = 0;
    
    for(i = 1; i < num_files; i++) {

        if(is_file(argv[i])) {
            files_paths[real_file_count++] = argv[i];
        }
    }

    

    // from this point, we have in files_paths array all files.
    int num_workers = real_file_count < 20 ? real_file_count : 20;//(int) num_files*0.2; // 20% of the files MAGIC NUMBER!!!!!!!!!!!!!!
    
    char * worker_parameters[] = { "./worker", NULL }; // parameters for execve
    int workers_fds[2][num_workers]; //matrix of workers file descriptors
    
    for(i = 0; i < num_workers; i++) {
        //Pipe that sends files to worker
        int pipe_files[2]; // 0 lectura 1 escritura
        
        //Pipe that recieves data from worker
        int pipe_data[2];

        //Creating pipes
        if(pipe(pipe_files) == -1 || pipe(pipe_data) == -1) {
            error_call("pipe call failed",1);
        }
          //Saving file descriptors
        workers_fds[READ][i] = pipe_data[READ];
        workers_fds[WRITE][i] = pipe_files[WRITE];

        // close(pipe_data[WRITE]);
        // close(pipe_files[READ]);

        //Creating worker
        int worker_fork = fork();
        
        if(worker_fork == -1) {
            error_call("pipe call failed",1);
        }
        

        //Closing FD from child and changing stdin and stdout
        if(worker_fork == CHILD) {
            int j;
            for(j=0; j<i;j++){
                close(workers_fds[READ][j]);
                close(workers_fds[WRITE][j]);
            }
            close(0); // From now on, recieving information from the pipe instead od stdin
            dup(pipe_files[READ]);
            close(1); // form now on, sending information to the pipe instead of stdout
            dup(pipe_data[WRITE]);
            close(pipe_files[READ]);
            close(pipe_data[WRITE]);
            close(pipe_data[READ]);
            close(pipe_files[WRITE]);
            execve("./worker", worker_parameters, 0);
        }
        
        close(pipe_files[READ]); //Closing pipes ends that the proccess is not going to use.
        close(pipe_data[WRITE]);
    }

    printf("Cantidad de workers: %d\n", num_workers);
    
    //INICIALMENTE MANDAMOS A CADA UNO DE LOS WORKERS 1 TRABAJO
    int file_to_send;
    for(file_to_send = 0; file_to_send < num_workers; file_to_send++) {
       if( write(workers_fds[WRITE][file_to_send], files_paths[file_to_send], strlen(files_paths[file_to_send])) == -1) {
            error_call("Write failed", 1);
       };
    
    }
    int j;
    char buffer [300] = {0};
    for(j = 0; j < num_workers; j++) {
        int n = read(workers_fds[READ][j],buffer,300 );
        buffer[n]=0;
        printf("%s \n",buffer);
    }

    return 0;
}

char is_file(char * path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void error_call(char * message_error, int return_number) {
    perror(message_error);
    exit(return_number);
}