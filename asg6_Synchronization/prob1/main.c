#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define POSIX_SEM_NAMED "/name_example"


static volatile int glob = 0;
sem_t* sem;

static void* func_thread(void *arg)
{
    char *s = (char *)arg;
    char write_data[10];

    int fd = open(s, O_WRONLY);

    if(fd == -1) {
        printf("Fail to open file %s", s);
        exit(1);
    }
    sem_wait(sem);
    for(int i = 0; i < 100000; i++) 
        glob++;
    sprintf(write_data, "%d", glob);
    write(fd, write_data, strlen(write_data));
    close(fd);
    sem_post(sem);
    return NULL;
}


int main(int argc, char **argv)
{
    int ret = -1;
    int current_val;

    pthread_t thread1, thread2;

    if(argc != 3) {
        printf("Program needs 2 name of file\n");
        return -1;
    }

    sem = sem_open(POSIX_SEM_NAMED, O_CREAT | O_EXCL, 0666, 1);
    if(sem == SEM_FAILED) {
        printf("Failed to open semaphore\n");
        return -1;
    }

    sem_getvalue(sem, &current_val);
    printf("Value of semaphoree: %d", current_val);
    ret = pthread_create(&thread1, NULL, func_thread, (void *)argv[1]);
    if(ret == -1) {
        printf("Failed to create thread\n");
        return -1;
    }
    ret = pthread_create(&thread2, NULL, func_thread, (void *)argv[2]);
    if(ret == -1) {
        printf("Failed to create thread\n");
        return -1;
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    sem_close(sem);
    sem_unlink(POSIX_SEM_NAMED);
    
    return 0;
}