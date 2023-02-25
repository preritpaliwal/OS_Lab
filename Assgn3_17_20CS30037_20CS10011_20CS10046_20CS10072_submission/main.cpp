#include<iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <set>
#include <unistd.h>
#include <wait.h>

using namespace std;

#define FILEPATH "facebook_combined.txt"
#define SHM_EDGE_SZ 150000


int main(){

    // create shared memory for storing the graph as a set of edges
    key_t key1 = ftok(FILEPATH, 37);
    int shmid1 = shmget(key1, SHM_EDGE_SZ*sizeof(int), IPC_CREAT | 0666);

    if (shmid1 == -1){
        perror("Error while creating shared memory.\n");
        exit(EXIT_FAILURE);
    }

    key_t key2 = ftok(FILEPATH, 38);
    int shmid2 = shmget(key2, SHM_EDGE_SZ*sizeof(int), IPC_CREAT | 0666);

    if (shmid2 == -1){
        perror("Error while creating shared memory.\n");
        exit(EXIT_FAILURE);
    }

    // attach to the shared memory
    int *shm1 = (int*) shmat(shmid1, NULL, 0);
    if (shm1 == (int*) -1){
        perror("Error while attaching to shared memory.\n");
        exit(EXIT_FAILURE);
    }

    int *shm2 = (int*) shmat(shmid2, NULL, 0);
    if (shm2 == (int*) -1){
        perror("Error while attaching to shared memory.\n");
        exit(EXIT_FAILURE);
    }

    // open the text file to read the graph and store it in the shared memory
    FILE *fp = fopen(FILEPATH, "r");
    if (fp == NULL){
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }

    // read the file line by line
    char line[50];
    int num_edges = 0, num_nodes = 0, node1, node2;
  
    // create a set of nodes
    set<int> nodes;

    shm1[0] = num_nodes; 
    shm1[1] = num_edges;
    shm2[0] = num_nodes;
    shm2[1] = num_edges;

    while(fgets(line, sizeof(line), fp) != NULL){
        sscanf(line, "%d %d", &node1, &node2);

        nodes.insert(node1); nodes.insert(node2);

        num_edges++;
        shm1[num_edges + 1] = node1;
        shm2[num_edges + 1] = node2;

        // update the number of nodes
        num_nodes = nodes.size();
        shm1[0] = num_nodes;
        shm2[0] = num_nodes;

        // update the number of edges
        shm1[1] = num_edges;
        shm2[1] = num_edges;
    }

        
    // close the file
    fclose(fp);
    printf("Number of nodes: %d, Number of edges: %d\n", num_nodes, num_edges);

    // detach from the shared memory
    shmdt(shm1);
    shmdt(shm2);

    // fork the producer and consumer processes
    pid_t producer_pid = fork();

    if (producer_pid == 0){

        execl("./prod", "./prod", NULL);
    }

    else{

        // fork 10 consumer processes
        for (int i = 0; i < 10; i++){
            pid_t consumer_pid = fork();
            char *cons_idx = (char*) malloc(sizeof(char));
            sprintf(cons_idx, "%d", i);
            if (consumer_pid == 0){
                
                execl("./cons", "./cons", cons_idx, NULL);
            }
        }

        // block the parent process until all the child processes are done
        for (int i = 0; i < 11; i++){
            wait(NULL);
        }

        // // wait for the producer to finish
        // wait(NULL);

        // delete the shared memory
        shmctl(shmid1, IPC_RMID, NULL);
        shmctl(shmid2, IPC_RMID, NULL);

    }


}