// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "lib.h"

int main(int argc, char * argv[]) {  
    char * shared_memory_name;
    char * rdwr_sem_name;
    char * signal_sem_name;
    if(argc > 1) { 
        // We receive arguments as parameters
        if(argc != 4){
            error_call("You must send exactly 3 arguments\n",1);
        }
        
        int size_of_shared_memory_name = BUFFER;
        int size_of_rdwr_sem_name = BUFFER;
        int size_of_signal_sem_name = BUFFER;
        // Creating the shm and semaphores name.
        shared_memory_name = malloc(size_of_shared_memory_name);
        rdwr_sem_name = malloc(size_of_rdwr_sem_name);
        signal_sem_name = malloc(size_of_signal_sem_name);

        if(shared_memory_name != NULL && rdwr_sem_name != NULL && signal_sem_name != NULL) {
            strncpy(shared_memory_name, argv[1], size_of_shared_memory_name);
            strncpy(rdwr_sem_name, argv[2], size_of_rdwr_sem_name);
            strncpy(signal_sem_name, argv[3], size_of_signal_sem_name);
        } else {
            error_call("Malloc failed", 1);
            exit(1);
        }
        
    } else { // We receive arguments from pipe or we wait for the user to send them
        shared_memory_name = NULL;
        rdwr_sem_name = NULL;
        signal_sem_name = NULL;
        size_t len = 0;
        // Getting shared memory name
        getline(&shared_memory_name, &len, stdin);
        // Getting read and write semaphore name
        len = 0;
        getline(&rdwr_sem_name, &len, stdin);
        // Getting signal semaphore name
        len = 0;
        getline(&signal_sem_name,&len,stdin);
        if(shared_memory_name != NULL && rdwr_sem_name != NULL && signal_sem_name != NULL) {
            // changes the '\n' for '\0' -> end of string
            shared_memory_name[strcspn(shared_memory_name, "\n")] = '\0';
            rdwr_sem_name[strcspn(rdwr_sem_name, "\n")] = '\0';
            signal_sem_name[strcspn(signal_sem_name, "\n")] = '\0';
        } else {
            error_call("Could not get the name of the shared memory or semaphore.",1);
            exit(1);
        } 
    }
    
    // Open shared memory
    
    int shm_fd;
    if((shm_fd = shm_open(shared_memory_name, O_RDWR, 0777)) == ERROR) {
        perror("opening the shared memory");
        exit(1);
    }
    
    Response * pointer_to_shm = (Response *) mmap(NULL, MAX_FILES * sizeof(Response), PROT_READ, MAP_SHARED, shm_fd, 0);
    Response * next_response = pointer_to_shm;

    // Open signal semaphore
    sem_t * signal_sem;
    if (( signal_sem = sem_open(signal_sem_name, O_RDONLY, S_IRUSR, 0)) == SEM_FAILED) {
        error_call("Error in opening semaphore", 1);
        exit(1);
    }
    // Open semaphore
    sem_t * rdwr_sem;
    if((rdwr_sem = sem_open(rdwr_sem_name, O_RDONLY, S_IRUSR, 0)) == SEM_FAILED) {
        error_call("Could not open semaphore",1);
        exit(1);
    }
    

    // Turning the semaphore to 0 so the wait in application blocks
    sem_wait(signal_sem);
    
    sem_wait(rdwr_sem);
    while((*next_response).pid > 0) {
        // Waiting for semaphore -> if it is 0, wait until it is 1
        // Semaphore is 1 -> we can read from shared memory
        print_process_information(*next_response);
        sem_wait(rdwr_sem);
        next_response ++;
    }
    

    // Closing shared memory
    //close_shared_memory(pointer_to_shm, MAX_FILES * sizeof(Response), shared_memory_name, shm_fd);
    

    if(munmap(pointer_to_shm, MAX_FILES * sizeof(Response)) == -1) {
        error_call("Error on unmaping the shared memory", 1);
    }

    sem_close(rdwr_sem);

    
    sem_post(signal_sem);
    sem_close(signal_sem);



    // Freeing memory
    free(shared_memory_name);
    free(rdwr_sem_name);
    free(signal_sem_name);
    return 0;
}