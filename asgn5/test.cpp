#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t sem;  // declare semaphore

void* thread_func(void* arg)
{
    sem_wait(&sem);  // wait for semaphore
    printf("Thread %ld received signal.\n", (long)arg);
    pthread_exit(NULL);
}

int main()
{
    pthread_t threads[3];
    sem_init(&sem, 0, 0);  // initialize semaphore with value 0

    for (long i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, thread_func, (void*)i);
    }

    sleep(5);  // allow some time for threads to start

    sem_post(&sem);  // signal all waiting threads

    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&sem);  // destroy semaphore
    return 0;
}
