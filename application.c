#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "application.h"
#include <sys/stat.h>
#include <string.h>
#include <sys/select.h>
#include "information.h"


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
    int num_workers = real_file_count < 5 ? real_file_count : 5;//(int) num_files*0.2; // 20% of the files MAGIC NUMBER!!!!!!!!!!!!!!
    
    char * worker_parameters[] = { "./worker", NULL }; // parameters for execve
    int workers_fds[2][num_workers]; //matrix of workers file descriptors
    
    int first_amount = (int) (0.2*real_file_count/num_workers);
    if(first_amount == 0) {
        first_amount = 1;
    } 

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
    printf("%d\n", first_amount);

    char pending_jobs[num_workers];
    
    for (i = 0; i < num_workers; i++) {
        pending_jobs[i] = 0;
    }

    
    
    //INICIALMENTE MANDAMOS A CADA UNO DE LOS WORKERS first_amount TRABAJOS
    int file_to_send = 0;
    int k;
    int p;
    for (p = 0; p < num_workers; p++){
        for(k = 0; k < first_amount; k++) {
            if(write(workers_fds[WRITE][p], files_paths[file_to_send], strlen(files_paths[file_to_send])) == -1) {
                error_call("Write failed", 1);
            }

            char eof = '\n';           
            write(workers_fds[WRITE][p], &eof,sizeof(eof));
            file_to_send++;
            pending_jobs[p]++;
        }
    }



    // SELECT
    // int j;
    // char buffer [300] = {0};
    // for(j = 0; j < num_workers; j++) {
    //     int n = read(workers_fds[READ][j],buffer,300 );
    //     buffer[n]=0;
    //     printf("%s \n",buffer);
    // }

    fd_set read_fds; // set with the read file descriptors
    int max_fd = get_max_from_array(workers_fds[READ],num_workers);
    int ready_fds; // to capture errors.
    Response response;
    printf("%d ACA\n", file_to_send);

   


    while(file_to_send <= real_file_count) {
        FD_ZERO(&read_fds); // restore values of fds that are ready to read.
        for(i = 0; i<num_workers; i++) {
            FD_SET(workers_fds[READ][i], &read_fds);
        }

        ready_fds = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        printf("Hay %d worker libres\n",ready_fds);
        if(ready_fds == -1) {
            perror("select");
            exit(1);
        }

        for(i = 0; i < num_workers; i++) {
            if (FD_ISSET(workers_fds[READ][i], &read_fds)){
                //READ en worker_fds[READ][i] es todo legal
                // Leo lo del worker
            
                int bytes_read = read(workers_fds[READ][i], &response, sizeof(response));
                if (bytes_read == -1) {
                    error_call("Error on read", 1);
                } else {
                    printf("-------------------\n");
                    printf("PID: %d\n",response.pid);
                    printf("name: %s\n",response.name);
                    printf("md5: %s\n",response.md5);
                    printf("-------------------\n");


                }
                // Si no le quedan cosas para hacer hago el write y aumento los trabajos del worker
                
            }
        }

    }

    return 0;
}

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
