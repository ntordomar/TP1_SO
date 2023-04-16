#include "lib.h"

/* --- SEMAPHORE ERRORS --- */
sem_t * create_semaphore(char * name) {
    sem_t * semaphore;
    if((semaphore = sem_open(name, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR, 0) ) == SEM_FAILED) {
        perror("Creating semaphore");
        exit(ERR_CREATE_SEM);
    }
    return semaphore;
}
