#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t mutex, sem;  // declare semaphores

void* thread_func(void* arg)
{
    sem_wait(&mutex);  // acquire mutex lock

    // critical section
    printf("Thread %ld is waiting.\n", (long)arg);
    sem_post(&mutex);  // release mutex lock

    sem_wait(&sem);  // wait for semaphore
    printf("Thread %ld received signal.\n", (long)arg);
    pthread_exit(NULL);
}

int main()
{
    pthread_t threads[3];
    sem_init(&mutex, 0, 1);  // initialize mutex semaphore with value 1
    sem_init(&sem, 0, 0);  // initialize counting semaphore with value 0

    for (long i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, thread_func, (void*)i);
    }

    sleep(5);  // allow some time for threads to start

    sem_wait(&mutex);  // acquire mutex lock
    sem_post(&sem);  // signal all waiting threads
    sem_post(&mutex);  // release mutex lock

    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&mutex);  // destroy mutex semaphore
    sem_destroy(&sem);  // destroy counting semaphore

    

    return 0;
}
