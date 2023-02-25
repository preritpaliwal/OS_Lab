#include<iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>

using namespace std;

// class Graph{

//     public:
//         int num_nodes;
//         int num_edges;
//         // set<int> nodes; // set to store nodes
//         // vector<pair<int, int>> edges; // vector to store an edge as a pair of nodes

//         // int * to store nodes
//         int *nodes;
//         // int * to store edges
//         int *edges_first;
//         int *edges_second;

        

//     // constructor
//     Graph(int num_nodes = 0, int num_edges = 0){
//         this->num_nodes = num_nodes;
//         this->num_edges = num_edges;
//         nodes = (int*) malloc(1*sizeof(int));
//         edges_first = (int*) malloc(1*sizeof(int));
//         edges_second = (int*) malloc(1*sizeof(int));
//     }

//     // destructor
//     ~Graph(){
//         free(nodes);
//         free(edges_first);
//         free(edges_second);
//     }

//     // add node to the graph
//     void add_node(int node){
        
//         // only add if node is not already present
//         for (int i = 0; i < num_nodes; i++){
//             if (nodes[i] == node){
//                 return;
//             }
//         }

//         // add node to the nodes array
//         if (num_nodes == 0){
//             nodes[0] = node;
//             num_nodes++;
//         }
//         else{
//             nodes = (int*) realloc(nodes, (num_nodes+1)*sizeof(int));
//             nodes[num_nodes] = node;
//             num_nodes++;
//         }

//     }

//     // add edge to the graph
//     void add_edge(int node1, int node2){
//         add_node(node1);
//         add_node(node2);

//         // add edge to the edges array
//         if (num_edges == 0){
//             edges_first[0] = node1;
//             edges_second[0] = node2;
//         }
//         else{
//             edges_first = (int*) realloc(edges_first, (num_edges+1)*sizeof(int));
//             edges_second = (int*) realloc(edges_second, (num_edges+1)*sizeof(int));
//             edges_first[num_edges] = node1;
//             edges_second[num_edges] = node2;
//         }
//         num_edges++;

//     }

//     // function to calculate the size of the graph object
//     int get_size(){
//         int size = 0;
//         size += num_nodes*sizeof(int);
//         size += num_edges*sizeof(int);
//         size += num_edges*sizeof(int);
//         return size;
//     }

// };

int main(){

    int *edge_first = (int *)malloc(1*sizeof(int));
    int *edge_second = (int *)malloc(1*sizeof(int));
    int num_edges = 0;

    // read the contents of the input file
    FILE *fp = fopen("facebook_combined.txt", "r");
    if (fp == NULL){
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }

    // read the file line by line
    char line[100];
    while (fgets(line, sizeof(line), fp) != NULL){
        int node1, node2;
        sscanf(line, "%d %d", &node1, &node2);
        // fb_graph.add_edge(node1, node2);
        if (num_edges == 0){
            edge_first[0] = node1;
            edge_second[0] = node2;
            num_edges++;
        }
        else{
            edge_first = (int*) realloc(edge_first, (num_edges+1)*sizeof(int));
            edge_second = (int*) realloc(edge_second, (num_edges+1)*sizeof(int));
            edge_first[num_edges] = node1;
            edge_second[num_edges] = node2;
            num_edges++;
        }

    }

    // close the file
    fclose(fp);

    printf("Number of edges: %d\n", num_edges);

    // use ftok() to generate unique key
    key_t key = ftok("facebook_combined.txt", 37);
    printf("Key: %d\n", key);

    size_t tot_size = sizeof(int) + 2*(num_edges*sizeof(int));

    printf("Total size: %ld\n", tot_size);
    
    // shmget returns an identifier in shmid
    int shmid = shmget(key, tot_size, IPC_CREAT | 0666);

    if (shmid < 0){
        perror("shmget");
        exit(1);
    }

    // shmat to attach to shared memory
    char *shm = (char*) shmat(shmid, NULL, 0);
    if (shm == (char *) -1){
        perror("shmat");
        exit(1);
    }

    // Attach the variables to the shared memory
    int *num_edges_int = (int*) shm;
    *num_edges_int = num_edges;
    
    // printf("Number of edges: %d\n", *num_edges_int);
    printf("Number of edges: %d\n", *(int*)shm);

    shm += sizeof(int);

    int *edge_first_int = (int*) shm;
    for (int i = 0; i < num_edges; i++){
        edge_first_int[i] = edge_first[i];
    }

    shm += num_edges*sizeof(int);

    int *edge_second_int = (int*) shm;
    for (int i = 0; i < num_edges; i++){
        edge_second_int[i] = edge_second[i];
    }
    shm += num_edges*sizeof(int);

    // // detach from shared memory
    shmdt(shm);

    return 0;
}