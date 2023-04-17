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
        
        // Creating the shm and semaphores name.
        shared_memory_name = malloc(BUFFER);
        rdwr_sem_name = malloc(BUFFER);
        signal_sem_name = malloc(BUFFER);

        //We use strncpy because its a safe function that allows us to copy exactly n characters without possibility of overflow
        if(shared_memory_name == NULL || rdwr_sem_name == NULL || signal_sem_name == NULL) {
            error_call("Malloc failed", 1);
        } 

        strncpy(shared_memory_name, argv[1], BUFFER);
        strncpy(rdwr_sem_name, argv[2], BUFFER);
        strncpy(signal_sem_name, argv[3], BUFFER);
        
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
            error_call("Could not get the name of the shared memory or semaphores.", ERROR);
        } 
    }
    printf("hola\n");
    // Open shared memory
    int shm_fd = 0;
    Response * pointer_to_shm = open_shared_memory(shared_memory_name, &shm_fd);
    Response * next_response = pointer_to_shm;

    // Open signal semaphore
    sem_t * signal_sem = open_semaphore(signal_sem_name, 0);

    // Open read and write semaphore
    sem_t * rdwr_sem = open_semaphore(rdwr_sem_name, 0);
       

    // Turning the semaphore to 0 so the wait in application blocks
    sem_wait(signal_sem);

    // Reading from shared memory
    sem_wait(rdwr_sem);
    while((*next_response).pid > 0) {
        // Waiting for semaphore -> if it is 0, wait until it is 1
        // Semaphore is 1 -> we can read from shared memory
        print_process_information(*next_response);
        sem_wait(rdwr_sem);
        next_response ++;
    }

    // Unmaping shared memory
    unmap_shared_memory(pointer_to_shm, &shm_fd);

    // Closing read and write semaphore
    close_semaphore(rdwr_sem);

    // Letting application know that we are done reading
    close_semaphore(signal_sem);
    // Closing signal semaphore
    sem_close(signal_sem);

    // Freeing memory
    free(shared_memory_name);
    free(rdwr_sem_name);
    free(signal_sem_name);
    return 0;
}