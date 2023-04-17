#include "lib.h"
/* --- SHARED MEMORY ERRORS --- */
Response * create_shared_memory(char * name, int * shm_fd) {
    if(((*shm_fd) = shm_open(name, O_CREAT | O_RDWR, 0777)) == ERROR) { 
        perror("Creating shared memory");
        exit(ERR_CREATE_SHM);
    }

    // Setting size of shared memory
    if(ftruncate(*shm_fd, SHM_SIZE) == ERROR) {
        perror("Setting size of shared memory");
        exit(ERR_SET_SIZE_SHM);
    }

    //Mapping shared memory
    Response * pointer_to_shm;
    if((pointer_to_shm = (Response *) mmap(NULL, SHM_SIZE,PROT_WRITE, MAP_SHARED, (*shm_fd), 0)) == MAP_FAILED) {
        perror("Mapping shared memory");
        exit(ERR_MAP_SHM);
    }
    return pointer_to_shm;
}

/* --- SEMAPHORE ERRORS --- */
sem_t * create_semaphore(char * name, int value) {
    sem_t * semaphore;
    if((semaphore = sem_open(name, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, value) ) == SEM_FAILED) {
        perror("Creating semaphore");
        exit(ERR_CREATE_SEM);
    }
    return semaphore;
}



