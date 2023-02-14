#include <iostream>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

#define KEY 235
#define FILEPATH "facebook_combined.txt"
#define SHARED_MEM_SIZE 1e5 + 5

// #########################################
// ## Ashwani Kumar Kamal (20CS10011)     ##
// ## Pranav Nyati (20CS30037)            ##
// ## Prerit Paliwal (20CS10046)          ##
// ## Vibhu (20CS10072)                   ##
// ## Operating Systems Laboratory        ##
// ## Assignment - 4                      ##
// #########################################

/*
graph (integer pointer) is a shared memory segment of size 1e5 + 5 bytes
graph[0] stores the number of nodes in the graph
graph[i] stores the node number of the node connected to node i
*/

int *init_graph(key_t, int);
void insert_node(int *, int, int);

int main()
{
    key_t key = ftok(FILEPATH, KEY);
    key_t shmid = shmget(key, SHARED_MEM_SIZE, IPC_CREAT | 0666);
    int *graph = init_graph(shmid, SHARED_MEM_SIZE);

    insert_node(graph, 1, 2);

    shmdt(graph);
    return 0;
}

int *init_graph(key_t shmid, int n_nodes)
{
    int *graph = (int *)shmat(shmid, NULL, 0);
    graph[0] = n_nodes;
    return graph;
}

void insert_node(int *graph, int src, int dest)
{
    if (graph[dest] == -1)
        graph[dest] = src;
    else
    {
        int temp = graph[dest];
        graph[dest] = src;
        graph[src] = temp;
    }

    if (graph[src] == -1)
        graph[src] = dest;
    else
    {
        int temp = graph[src];
        graph[src] = dest;
        graph[dest] = temp;
    }
}