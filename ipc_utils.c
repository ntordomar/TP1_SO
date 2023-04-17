#include "lib.h"


/* --- SHARED MEMORY --- */
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

Response * open_shared_memory(char * name, int * shm_fd) {
    if(((*shm_fd) = shm_open(name, O_RDWR, 0777)) == ERROR) {
        perror("Opening shared memory");
        exit(ERR_OPEN_SHM);
    }

    //Mapping shared memory
    Response * pointer_to_shm;
    if((pointer_to_shm = (Response *) mmap(NULL, SHM_SIZE, PROT_READ, MAP_SHARED, (*shm_fd), 0)) == MAP_FAILED) {
        perror("Mapping shared memory");
        exit(ERR_MAP_SHM);
    }
    return pointer_to_shm;
}

void unmap_shared_memory(Response * pointer_to_shm, int * shm_fd) {
    if(munmap(pointer_to_shm, SHM_SIZE) == ERROR) {
        perror("Unmapping shared memory");
        exit(ERR_UNMAP_SHM);
    }
    close(*shm_fd);
}

void unlink_shared_memory(char * name) {
    if(shm_unlink(name) == ERROR) {
        perror("Unlinking shared memory");
        exit(ERR_UNLINK_SHM);
    }
}



/* --- SEMAPHORE --- */
sem_t * create_semaphore(char * name, int value) {
    sem_t * semaphore;
    if((semaphore = sem_open(name, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, value) ) == SEM_FAILED) {
        perror("Creating semaphore");
        exit(ERR_CREATE_SEM);
    }
    return semaphore;
}

sem_t * open_semaphore(char * name, int value) {
    sem_t * semaphore;
    if((semaphore = sem_open(name,  O_RDONLY, S_IRUSR, value)) == SEM_FAILED) {
        perror("Opening semaphore");
        exit(ERR_OPEN_SEM);
    }
    return semaphore;
}

void close_semaphore(sem_t * semaphore) {
    if(sem_close(semaphore) == ERROR) {
        perror("Closing semaphore");
        exit(ERR_CLOSE_SEM);
    }
}

void unlink_semaphore(char * name) {
    if(sem_unlink(name) == ERROR) {
        perror("Unlinking semaphore");
        exit(ERR_UNLINK_SEM);
    }
}
