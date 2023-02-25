#include<iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <set>
#include <vector>
#include <algorithm>
#include <time.h>
#include <unistd.h>
#include <string>

#define FILEPATH "facebook_combined.txt"
#define SHM_EDGE_SZ 150000
#define SHM_NODE_MAP_SZ 100

using namespace std;

// function to map the initial set of nodes to the ith consumer 
int map_initial_nodes(int *shm1, int *shm2, int num_nodes, int num_edges, int consumer_id, vector <int> &mapped_nodes){

    int num_nodes_per_consumer = num_nodes/10;
    int rem = num_nodes%10;

    // if the consumer id is less than the remainder, then it gets one more node
    if (consumer_id < rem){
        num_nodes_per_consumer++;

        for (int i = 0; i < num_nodes_per_consumer; i++)          // map the nodes to the consumer
            mapped_nodes.push_back(consumer_id*(num_nodes_per_consumer) + i);

    }

    // if the consumer id is greater than the remainder, then it gets the same number of nodes as the other consumers
    else{
        // map the nodes to the consumer
        for (int i = 0; i < num_nodes_per_consumer; i++){
            mapped_nodes.push_back(rem*(num_nodes_per_consumer + 1) + (consumer_id - rem)*num_nodes_per_consumer + i);
        }
    }

    string filename = "out/consumer_" + to_string(consumer_id) + ".txt";
    FILE *fp = fopen(filename.c_str(), "w");

    // write first the number of nodes and then the nodes themselves
    fprintf(fp, "Number of nodes mapped to consumer %d: %d\n", consumer_id, num_nodes_per_consumer);
    fprintf(fp, "Nodes mapped to consumer %d: \n", consumer_id);
    for (int i = 0; i < num_nodes_per_consumer; i++){
        fprintf(fp, "%d\n", mapped_nodes[i]);
    }
    fclose(fp);

    return 0;
}

// function to compute shortest path using Dijkstra's algorithm
int compute_shortest_path();


int main(int argc, char *argv[]){
    
    int consumer_id = atoi(argv[1]);      // get the consumer id to know which nodes to map
    key_t key1 = ftok(FILEPATH, 37), key2 = ftok(FILEPATH, 38);

    int shmid1 = shmget(key1, SHM_EDGE_SZ*sizeof(int), IPC_CREAT | 0666);
    if (shmid1 == -1){
        perror("Error while creating shared memory.\n");
        exit(EXIT_FAILURE);
    }

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

    sleep(30); // sleep for 30 seconds to allow the producer to finish writing to the shared memory
    // consumer wakes up after every 30 seconds to process the updated graph

    int num_nodes = shm1[0], num_edges = shm1[1];     // get the number of nodes and edges
    vector <int> mapped_nodes;  // vector to store the nodes mapped to the ith consumer process

    map_initial_nodes(shm1, shm2, num_nodes, num_edges, consumer_id, mapped_nodes);     // map the initial set of nodes to the ith consumer
    cout << "Consumer " << consumer_id << " has mapped " << mapped_nodes.size() << " nodes to itself." << endl;

    // function call to compute the initial shortest path (from mapped nodes of consumer i to all other nodes)
    //compute_shortest_path(shm1, shm2, num_nodes, num_edges, consumer_id, mapped_nodes);

    sleep(30);

    key_t node_map_key = ftok(FILEPATH, 40 + consumer_id);
    int node_map_shmid = shmget(node_map_key, SHM_NODE_MAP_SZ*sizeof(int), IPC_CREAT | 0666);

    if (node_map_shmid == -1){
        perror("Error while creating shared memory.\n");
        exit(EXIT_FAILURE);
    }
    int *node_map_shm = (int*) shmat(node_map_shmid, NULL, 0);      // attach to the shared memory

    // while loop to keep the consumer process running
    while(1){
    

        // get the updated number of nodes and edges
        int num_nodes_new = shm1[0], num_edges_new = shm1[1];
        cout << "Consumer:Number of nodes: " << num_nodes_new << endl;
        cout << "Consumer:Number of edges: " << num_edges_new << endl;

        if (num_nodes_new == num_nodes && num_edges_new == num_edges){  // if no change made, then sleep for 30 seconds and wake up again
            sleep(30);
            continue;
        }

        // get the updated set of nodes mapped to the ith consumer from the shared memory and append it to the mapped_nodes vector
        int num_new_nodes = node_map_shm[0];
        for (int i = 1; i <= num_new_nodes; i++){
            mapped_nodes.push_back(node_map_shm[i]);
        }

        // write the updated set of nodes mapped to the ith consumer to the output file
        string filename = "out/consumer_" + to_string(consumer_id) + ".txt";
        FILE *fp = fopen(filename.c_str(), "a");
        fprintf(fp, "Number of new nodes mapped to consumer %d: %d\n", consumer_id, num_new_nodes);
        fprintf(fp, "New nodes mapped to consumer %d: \n", consumer_id);
        for (int i = 1; i <= num_new_nodes; i++){
            fprintf(fp, "%d\n", node_map_shm[i]);
        }

        fclose(fp);

        // function call to compute the shortest path (from mapped nodes of consumer i to all other nodes)
        //compute_shortest_path(shm1, shm2, num_nodes, num_edges, consumer_id, mapped_nodes);

        num_nodes = num_nodes_new;
        num_edges = num_edges_new;

        // 

        sleep(30);
    }

    return 0;

}