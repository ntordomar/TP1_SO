// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "information.h"
#include "application.h"

int main(int argc, char *argv[]) {
    int aux = 0;
    int num_files = argc; 
    char * files_paths[num_files];

    int i;
    int real_file_count = 0;


    //SHARED MEMORY
    
    // Creating shared memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if(shm_fd == -1) {
        error_call("Error on shm_open", 1);
    }

    // Setting size of shared memory
    if(ftruncate(shm_fd, MAX_FILES * sizeof(Response)) == -1) {
        error_call("Error on ftruncate", 1);
    }

    // Mapping shared memory
    Response * pointer_to_shm = (Response *) mmap(NULL, MAX_FILES * sizeof(Response),PROT_WRITE, MAP_SHARED, shm_fd, 0);
    //SHARED MEMORY 

    //SEMPHORES  sem_post(sem*)  sem_wait(sem*)
    sem_t * semaphore = sem_open(SEM_NAME, O_CREAT, 0644, 0);
    int val = 0;
    sem_getvalue(semaphore, &val);
    // printf("%d\n", val);

    // sending to standard output the info to connect with the shared memory and semaphores
    // the standard output can be either the pipe or the terminal.
    printf("%s\n%s\n","/app_view_shm", "/app_view_sem");
    sleep(10);

    for(i = 1; i < num_files; i++) {
        if(is_file(argv[i])) {
            char * buff = malloc(strlen(argv[i]) * 2); // We decided to duplicate the length of the string in case the file has a lot of spaces.
            if(buff == NULL){
                error_call("Could not allocate the necesary memory\n",1);
            }
            normalize_string(argv[i], buff);
            files_paths[real_file_count++] = buff;
        }
    }
    
    if(real_file_count == 0){
        // printf("no files recieved\n");
        return 0;
    }
    
    // from this point, we have in the files_paths array all files.
    int num_workers = real_file_count < MAX_CANT_OF_WORKERS ? real_file_count : MAX_CANT_OF_WORKERS;
    char * worker_parameters[] = { "./worker", NULL };  // parameters for execve
    int workers_fds[2][num_workers];                    // matrix of workers file descriptors
    
    int first_amount = (int) (0.2*real_file_count/num_workers);
    if(first_amount == 0) {
        first_amount = 1;
    } 
    // Creating all workers and their pipes.
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



    // Creating the file of the response.
    FILE * file_of_information = fopen("response.txt", "w");

    // Using select to check which workers sent the information.    
    fd_set read_fds; // Set with the read file descriptors
    int max_fd = get_max_from_array(workers_fds[READ],num_workers);
    int ready_fds; // How many workers are ready to read.
    Response response; // Struct with the information of the worker answer.
    int amount_read = 1;
    int aux_jobs;
    while(amount_read <= real_file_count) {
        FD_ZERO(&read_fds); // Restore values of fds that are ready to read.
        for(i = 0; i<num_workers; i++) {
            // Addig the file descriptors to the set.
            FD_SET(workers_fds[READ][i], &read_fds);
        }
        
        // Checking which workers are ready to read.
        ready_fds = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if(ready_fds == -1) {
            error_call("Select failed.", 1);
        }

        int pepe = 0;
        sem_getvalue(semaphore, &pepe);
        // printf("%d\n", pepe);
        
        // Iterating each worker to see if its ready to read.
        for(i = 0; i < num_workers; i++) {
            // Checking whether if a worker is blocked for reading.
            if(FD_ISSET(workers_fds[READ][i], &read_fds)) { 
                
                if(read(workers_fds[READ][i], &response, sizeof(response)) == -1) {
                    error_call("Error on read", 1);
                } else {
                    amount_read ++;
                    aux_jobs = pending_jobs[i];
                    pending_jobs[i] = aux_jobs -1;
                    fprintf(file_of_information,"-------------------\n PID: %d\n name: %s md5: %s\n-------------------\n",response.pid,response.name,response.md5 );          
                    pointer_to_shm[aux++] = response;
                    sem_post(semaphore);

                    val = 0;
                    sem_getvalue(semaphore, &val);
                    // printf("%d\n", val);
                    
                    // print_process_information(response);
                    // Sending information to view

                }
                if(!pending_jobs[i]) { //We keep track if a worker is doing any jobs before sending a new one
                    if(file_to_send < real_file_count) { 
                        if(write(workers_fds[WRITE][i], files_paths[file_to_send], strlen(files_paths[file_to_send])) == -1) {
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
        
    // Sending finish signal to view
    Response finish;
    finish.pid = -1;
    pointer_to_shm[aux] = finish;
    sem_post(semaphore);
    // Closing semaphore
    sem_close(semaphore);
    // sem_destroy(semaphore);

    // Freeing memory thats being used to store the paths.
    for(i = 0; i < real_file_count; i++) {
        free(files_paths[i]);
    }

    fclose(file_of_information);
    
    return 0;
}

