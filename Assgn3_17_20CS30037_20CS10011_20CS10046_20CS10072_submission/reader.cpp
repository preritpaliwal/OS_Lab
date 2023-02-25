#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>

#define FILEPATH "facebook_combined.txt"
#define SHM_EDGE_SZ 150000


using namespace std;

int main(){

    key_t key1 = ftok(FILEPATH, 37);
    key_t key2 = ftok(FILEPATH, 38);


    int shmid1 = shmget(key1, SHM_EDGE_SZ*sizeof(int), IPC_CREAT | 0666);
    if (shmid1 < 0){
        perror("shmget");
        exit(1);
    }

    int shmid2 = shmget(key2, SHM_EDGE_SZ*sizeof(int), IPC_CREAT | 0666);
    if (shmid2 < 0){
        perror("shmget");
        exit(1);
    }

    // attach to the shared memory
    int *shm1 = (int*) shmat(shmid1, NULL, 0);
    if (shm1 == (int*) -1){
        perror("shmat");
        exit(1);
    }

    int *shm2 = (int*) shmat(shmid2, NULL, 0);
    if (shm2 == (int*) -1){
        perror("shmat");
        exit(1);
    }

    // read the graph from the shared memory
    printf("Number of nodes: %d\n", shm1[0]);
    printf("Number of nodes: %d\n", shm2[0]);
    printf("Number of edges: %d\n", shm1[1]);
    printf("Number of edges: %d\n", shm2[1]);

    FILE *fp = fopen("output.txt", "w");
    if (fp == NULL){
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < shm1[1]; i++){
        fprintf(fp, "%d %d\n", shm1[2 + i], shm2[2 + i]);
    }


    // detach from shared memory
    shmdt(shm1);
    shmdt(shm2);

    // destroy the shared memory
    shmctl(shmid1, IPC_RMID, NULL);
    shmctl(shmid2, IPC_RMID, NULL);

    return 0;
}