#include "lib.h"

/* --- PIPES AND PROCESSES --- */
void create_pipe(int * pipe_fd) {
    if(pipe(pipe_fd) == ERROR) {
        error_call("Creating pipe", ERR_CREATE_PIPE);
    }
}

void close_pipe(int * pipe, int fd) {
    if(close(pipe[fd]) == ERROR) {
        error_call("Closing pipe", ERR_CLOSE_PIPE);
    }
}

int create_process() {
    int pid;
    if((pid = fork()) == ERROR) {
        error_call("Creating process", ERR_CREATE_PROCESS);
    }
    return pid;
}

void start_process(char * path_name, char * parameters[]) {
    if(execve(path_name, parameters, 0) == ERROR) {
        error_call("Starting process", ERR_START_PROCESS);
    }
}

void select_process(int max_fd, fd_set * read_fds) {
    if(select(max_fd + 1, read_fds, NULL, NULL, NULL) == ERROR) {
        error_call("Selecting process", ERR_SELECT_PROCESS);
    }
}

void read_process(int fd, Response * response) {
    if(read(fd, response, sizeof(Response)) == ERROR) {
        error_call("Reading process", ERR_READ_PROCESS);
    }
}

void write_process(int fd, char * file_path) {
    if(write(fd, file_path, strlen(file_path)) == ERROR) {
        error_call("Writing process", ERR_WRITE_PROCESS);
    }
}


/* --- SHARED MEMORY --- */
Response * create_shared_memory(char * name, int * shm_fd) {
    if(((*shm_fd) = shm_open(name, O_CREAT | O_RDWR, 0777)) == ERROR) { 
        error_call("Creating shared memory", ERR_CREATE_SHM);
    }

    // Setting size of shared memory
    if(ftruncate(*shm_fd, SHM_SIZE) == ERROR) {
        error_call("Setting size of shared memory", ERR_SET_SIZE_SHM);
    }

    //Mapping shared memory
    Response * pointer_to_shm;
    if((pointer_to_shm = (Response *) mmap(NULL, SHM_SIZE,PROT_WRITE, MAP_SHARED, (*shm_fd), 0)) == MAP_FAILED) {
        error_call("Mapping shared memory", ERR_MAP_SHM);
    }
    return pointer_to_shm;
}

Response * open_shared_memory(char * name, int * shm_fd) {
    if(((*shm_fd) = shm_open(name, O_RDWR, 0777)) == ERROR) {
        error_call("Opening shared memory", ERR_OPEN_SHM);
    }

    //Mapping shared memory
    Response * pointer_to_shm;
    if((pointer_to_shm = (Response *) mmap(NULL, SHM_SIZE, PROT_READ, MAP_SHARED, (*shm_fd), 0)) == MAP_FAILED) {
        error_call("Mapping shared memory", ERR_MAP_SHM);
    }
    return pointer_to_shm;
}

void unmap_shared_memory(Response * pointer_to_shm, int * shm_fd) {
    if(munmap(pointer_to_shm, SHM_SIZE) == ERROR) {
        error_call("Unmapping shared memory", ERR_UNMAP_SHM);
    }
    close(*shm_fd);
}

void unlink_shared_memory(char * name) {
    if(shm_unlink(name) == ERROR) {
        error_call("Unlinking shared memory", ERR_UNLINK_SHM);
    }
}



/* --- SEMAPHORE --- */
sem_t * create_semaphore(char * name, int value) {
    sem_t * semaphore;
    if((semaphore = sem_open(name, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, value) ) == SEM_FAILED) {
        error_call("Creating semaphore", ERR_CREATE_SEM);
    }
    return semaphore;
}

sem_t * open_semaphore(char * name, int value) {
    sem_t * semaphore;
    if((semaphore = sem_open(name,  O_RDONLY, S_IRUSR, value)) == SEM_FAILED) {
        error_call("Opening semaphore", ERR_OPEN_SEM);
    }
    return semaphore;
}

void close_semaphore(sem_t * semaphore) {
    if(sem_close(semaphore) == ERROR) {
        error_call("Closing semaphore", ERR_CLOSE_SEM);
    }
}

void unlink_semaphore(char * name) {
    if(sem_unlink(name) == ERROR) {
        error_call("Unlinking semaphore", ERR_UNLINK_SEM);
    }
}

void unlink_exec_failed(char * sem1, char * sem2, char * shm){
    sem_unlink(sem1);
    sem_unlink(sem2);
    shm_unlink(shm);
}
