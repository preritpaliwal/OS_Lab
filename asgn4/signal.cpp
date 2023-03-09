#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 3

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int count = 0;

void *thread_function(void *arg) {
  int id = *(int *)arg;
  pthread_mutex_lock(&mutex);
  printf("Thread %d: waiting...\n", id);
  while (count < NUM_THREADS) {
    pthread_cond_wait(&cond, &mutex);
  }
  printf("Thread %d: finished!\n", id);
  pthread_mutex_unlock(&mutex);
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  pthread_t threads[NUM_THREADS];
  int thread_ids[NUM_THREADS];

  for (int i = 0; i < NUM_THREADS; i++) {
    thread_ids[i] = i;
    pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]);
  }

  printf("Main thread: sleeping...\n");
  sleep(5);
  printf("Main thread wakes up after 5 seconds.\n");

  pthread_mutex_lock(&mutex);
  count = NUM_THREADS;
  pthread_cond_signal(&cond); // Use pthread_cond_signal instead of pthread_cond_broadcast
  pthread_mutex_unlock(&mutex);

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  return 0;
}
