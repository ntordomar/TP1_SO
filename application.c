// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "lib.h"
#include "application.h"

int main(int argc, char *argv[]) {
  
    int num_files = argc; 
    char * files_paths[num_files];
    int i;

    //Unlinking shared memory and semaphores if a previous execution was interrupted
    unlink_exec_failed(RDWR_SEM, SIGNAL_SEM, SH_MEM);
    
    // Creating shared memory
    
    int shm_fd = 0;
    Response * pointer_to_shm = create_shared_memory(SH_MEM, &shm_fd);
   
    // Creating the semaphore of read and write files
    sem_t * rdwr_sem = create_semaphore(RDWR_SEM, 0);

    // Creating the signal semaphore (it will indicate when view has finished tasks)
    sem_t * signal_sem = create_semaphore(SIGNAL_SEM, 1);

    // sending to standard output the info to connect with the shared memory and semaphores
    // the standard output can be either the pipe or the terminal.
    printf("%s\n%s\n%s\n",SH_MEM, RDWR_SEM, SIGNAL_SEM);
    fflush(stdout);
    
    //Waiting for the user to start the view process
    sleep(5); 

    //On the function filter_normalize_files we loop through the argv array and filter only the ones which are files
    int real_file_count = filter_normalize_files(num_files, argv, files_paths);
    if(real_file_count == 0){
        error_call("no files recieved",0);
    }
    
    // from this point, we have in the files_paths array all files with the correct format.
    // We determine the amount of workers based on the real file count
    int num_workers = real_file_count < MAX_CANT_OF_WORKERS ? real_file_count : MAX_CANT_OF_WORKERS;
    char * worker_parameters[] = { "./worker", NULL };  // parameters for execve
    int workers_fds[2][num_workers];                    // matrix of workers file descriptors 
    
    //The first amount sent to each worker is also calculated based on the file count.
    int first_amount = (int) (0.2*real_file_count/num_workers);
    if(first_amount == 0) {
        first_amount = 1;
    } 
    int workers_pid [num_workers];
    for(i = 0; i<num_workers; i++){
        workers_pid[i] = 0;
    }
    
    // Creating all workers and their pipes.
    for(i = 0; i < num_workers; i++) { 
        //Pipe that sends files to worker
        int pipe_files[2] = {0};
        //Pipe that recieves data from worker
        int pipe_data[2] = {0};

        //Creating pipes
        create_pipe(pipe_files);
        create_pipe(pipe_data);

         //Saving file descriptors
        workers_fds[READ][i] = pipe_data[READ];
        workers_fds[WRITE][i] = pipe_files[WRITE];

        //Creating worker
        int worker_fork = create_process();
        
        //Closing FD from child and changing stdin and stdout
        if(worker_fork == CHILD) {
            workers_pid[i] = getpid();
            int j;
            // We delete pipes of previous workers.
            for(j=0; j<i;j++){
                close(workers_fds[READ][j]);
                close(workers_fds[WRITE][j]);
            }
            //Opening and closing desired fd before calling execve.
            manage_worker_pipes(pipe_files,pipe_data); 
            //Calling execve
            start_process(WORKER_PATH, worker_parameters);
        }
        //Closing pipes that the application proccess is not going to use. (read to the pipes of information, write from  the pipe of response)
        close_pipe(pipe_data, WRITE);
        close_pipe(pipe_files, READ);
    }

    // in this array we will save the amount of files each worker is processing
    char pending_jobs[num_workers]; 
    for (i = 0; i < num_workers; i++) {
        pending_jobs[i] = 0;
    }

    // the number of the next file to send
    int file_to_send = 0; 

    //Here we send 'first_ammount' of files to each worker.
    sending_first_files(&file_to_send, first_amount, workers_fds[WRITE],files_paths, pending_jobs, num_workers);



    // Creating the file of the response. 
    FILE * file_of_information =  create_file("response.txt","w");

    // These are variables we will use in the process of sending files to worker.   
    fd_set read_fds; // Set with the read file descriptors
    int max_fd = get_max_from_array(workers_fds[READ],num_workers);
    Response response; // Struct with the information of the worker answer.
    int amount_read = 1;
    int aux_jobs;
    int next_to_write_in_shm = 0;

    // We will start sending files to each worker.
    while(amount_read <= real_file_count) {
        
        FD_ZERO(&read_fds); // Restore values of fds that are ready to read.
        for(i = 0; i<num_workers; i++) {
            // Addig the file descriptors to the set.
            FD_SET(workers_fds[READ][i], &read_fds);
        }
        
        // Checking which workers are ready to be read.
        select_process(max_fd, &read_fds);
        
        
        // Iterating each worker to see if its ready to be read.
        for(i = 0; i < num_workers; i++) {

            if(FD_ISSET(workers_fds[READ][i], &read_fds)) { 
                // Reading the response from the worker.
                read_process(workers_fds[READ][i], &response);
                amount_read ++;
                aux_jobs = pending_jobs[i];
                pending_jobs[i] = aux_jobs -1;
                // sending information to 'response.txt' file
                fprintf(file_of_information,"-------------------\n PID: %d\n name: %s md5: %s\n-------------------\n",response.pid,response.name,response.md5 );          
                // Sending information to view
                pointer_to_shm[next_to_write_in_shm++] = response;
                sem_post(rdwr_sem);
                
                //We keep track if a worker is doing any jobs before sending a new one
                if(!pending_jobs[i]) { 
                    if(file_to_send < real_file_count) { 
                        // Sending file to worker
                        write_process(workers_fds[WRITE][i], files_paths[file_to_send]);
                        file_to_send++;
                        aux_jobs = pending_jobs[i];
                        pending_jobs[i] = aux_jobs + 1;
                    }
                }
            }
        }
    }

    // Killing workers
    for(i = 0; i < num_workers; i++) {
        kill(workers_pid[i], 0);
    }
        
    // Sending finish signal to view
    Response finish;
    // By setting the last pid to -1, the while loop in view will stop.
    finish.pid = -1;
    pointer_to_shm[next_to_write_in_shm] = finish;
    sem_post(rdwr_sem);
    
    // In case view is enabled, we wait until it finishes
    sem_wait(signal_sem);

    // Unmaping and unlinking shared memory
    unmap_shared_memory(pointer_to_shm, &shm_fd);
    unlink_shared_memory(SH_MEM);

    // Closing and unlinking both semaphores    
    close_semaphore(rdwr_sem);
    close_semaphore(signal_sem);
    unlink_semaphore(RDWR_SEM);
    unlink_semaphore(SIGNAL_SEM);
    
    // Freeing memory thats being used to store the paths.
    for(i = 0; i < real_file_count; i++) {
        free(files_paths[i]);
    }

    // closing response.txt
    fclose(file_of_information);

    return 0;
}

