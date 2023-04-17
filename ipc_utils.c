#include "lib.h"

/* --- PIPES AND PROCESSES --- */
void create_pipe(int * pipe_fd) {
    if(pipe(pipe_fd) == ERROR) {
        perror("Creating pipe");
        exit(ERR_CREATE_PIPE);
    }
}

void close_pipe(int * pipe, int fd) {
    if(close(pipe[fd]) == ERROR) {
        perror("Closing pipe");
        exit(ERR_CLOSE_PIPE);
    }
}

int create_process() {
    int pid;
    if((pid = fork()) == ERROR) {
        perror("Creating worker");
        exit(ERR_CREATE_PROCESS);
    }
    return pid;
}

void start_process(char * path_name, char * parameters[]) {
    if( execve(path_name, parameters, 0) == ERROR) {
        perror("Could not create a worker");
        exit(ERR_START_PROCESS);
    }
}

void select_process(int max_fd, fd_set * read_fds) {
    if(select(max_fd + 1, read_fds, NULL, NULL, NULL) == ERROR) {
        perror("Selecting process");
        exit(ERR_SELECT_PROCESS);
    }
}

void read_process(int fd, Response * response) {
    if(read(fd, response, sizeof(Response)) == ERROR) {
        perror("Reading process");
        exit(ERR_READ_PROCESS);
    }
}

void write_process(int fd, char * file_path) {
    if(write(fd, file_path, strlen(file_path)) == ERROR) {
        perror("Writing process");
        exit(ERR_WRITE_PROCESS);
    }
}


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
