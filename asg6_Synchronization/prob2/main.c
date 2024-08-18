#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
/******************************************************************************
 *                              DEFINE AND TYPEDEF 
******************************************************************************/
#define NUMBER_OF_THREAD 3
typedef struct main
{
    int loops;
    int thread_id;
} thread_data_t;

static int glob = 0;
static sem_t *sem;

static void *thread_func(void *arg)
{
    thread_data_t thread_data = *((thread_data_t *) arg);
    int loc, j;
    if (sem_wait(sem) == -1)
    {
        printf("sem_wait");
    }
    for (j = 0; j < thread_data.loops; j++) 
    {
        loc = glob;
        loc++;
        glob = loc;
    }
    printf("Thread %d - glob: %d\r\n", thread_data.thread_id, glob);
    if (sem_post(sem) == -1)
    {
        printf("sem_post");
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t thread[NUMBER_OF_THREAD];
    int s;

    /* Initialize a semaphore with the value 1 */
    thread_data_t thread_data[NUMBER_OF_THREAD];
    if (sem_init(sem, 0, 1) == -1)
        printf("sem_init");

    /* Create two threads that increment 'glob' */
    for(int i = 0; i< NUMBER_OF_THREAD; i++)
    {
        thread_data[i].loops = 100000;
        thread_data[i].thread_id = i + 1;
        s = pthread_create(&thread[i], NULL, thread_func, &thread_data);
        if (s != 0)
            printf("pthread_create index %i", i);
    }
    /* Wait for threads to terminate */
    for(int i = 0; i< NUMBER_OF_THREAD; i++)
    {
        s = pthread_join(thread[i], NULL);
        if (s != 0)
            printf("pthread_join");
    }
    printf("glob = %d\n", glob);
    exit(EXIT_SUCCESS);
}