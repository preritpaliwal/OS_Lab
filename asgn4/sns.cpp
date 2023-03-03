#include <iostream>
#include <pthread.h>
#include <time.h>
#include <unistd.h> // to use sleep
using namespace std;

// void *t1(void *vargp){
//     printf("thread started\n");
//     sleep(1);
//     printf("done with thread\n");
//     return NULL;
// }

enum ActionType { POST, COMMENT, LIKE };

struct Action{
    int *userId;
    int actionId;
    int actionType;
    time_t timeStamp;
};

struct Node{
    int number;
    int chorono;
    int degree;
    Action *wallQueue;
    Action *feedQueue;
    int *neighbors;
};

struct Graph{
    int num_edges;
    int num_nodes;
    Node *graphNodes;
}graph;


void initNode(Node *n,int num){
    n->number = num;
    n->chorono = 1;
    n->degree = 0;
    n->wallQueue = NULL;
    n->feedQueue = NULL;
    n->neighbors = NULL;
}

void addNeighbor(Node *n1,Node *n2){
    n1->degree++;
    n1->neighbors = (int *)realloc(n1->neighbors,n1->degree*(sizeof(int)));
    n2->degree++;
    n2->neighbors = (int *)realloc(n2->neighbors,n2->degree*(sizeof(int)));
    n1->neighbors[n1->degree -1] = n2->number;
    n2->neighbors[n2->degree -1] = n1->number;
}

void addEdge(Graph *graph,int n1,int n2){
    int posmax = max(n1,n2);
    if(posmax>=graph->num_nodes){
        cout<<"num of nodes"<<graph->num_nodes<<endl;
        graph->graphNodes = (Node *)realloc(graph->graphNodes,(posmax+1)*sizeof(Node));
        if(n1>graph->num_nodes){
            initNode(&(graph->graphNodes[n1]),n1);
        }
        if(n2>graph->num_nodes){
            initNode(&(graph->graphNodes[n2]),n2);
        }
        graph->num_nodes = posmax+1;
    }
    addNeighbor(&(graph->graphNodes[n1]),&(graph->graphNodes[n2]));
    graph->num_edges++;
}


int main()
{
    // pthread_t t_id;
    // printf("Before creating thread\n");
    // pthread_create(&t_id,NULL,t1,NULL);
    // pthread_join(t_id,NULL);
    // printf("Thread created\n");
    FILE *logFile;
    logFile = fopen("sns.log", "w");
    if (NULL == logFile)
    {
        printf("log file can't be opened \n");
        exit(EXIT_FAILURE);
    }

    // read the contents of the input file
    FILE *graphFile = fopen("musae_git_edges.csv", "r");
    if (graphFile == NULL){
        perror("Error while opening the graph file.\n");
        exit(EXIT_FAILURE);
    }

    graph.graphNodes = NULL;
    graph.num_edges = 0;
    graph.num_nodes = 0;
    char line[100];
    int idx = 0;
    while (fgets(line, sizeof(line), graphFile) != NULL){
        idx++;
        if(idx==1){
            continue;
        }
        cout<<idx<<endl;
        cout<<"line= "<<line<<endl;
        int node1, node2;
        sscanf(line, "%d,%d", &node1, &node2);
        cout<<"node1,node2: "<<node1<<","<<node2<<endl;
        addEdge(&graph,node1,node2);
    }
    // close the file
    fclose(graphFile);

    printf("Number of Edges: %d\n", graph.num_edges);
    printf("Number of Nodes: %d\n", graph.num_nodes);


    
    fclose(logFile);
    return 0;
}