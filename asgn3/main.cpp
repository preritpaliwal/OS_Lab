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

// #########################################
// ## Ashwani Kumar Kamal (20CS10011)     ##
// ## Pranav Nyati (20CS30037)            ##
// ## Prerit Paliwal (20CS10046)          ##
// ## Vibhu (20CS10072)                   ##
// ## Operating Systems Laboratory        ##
// ## Assignment - 4                      ##
// #########################################

typedef struct _node
{
    int data;
    struct _node *next;
} Node;

typedef struct
{
    Node *NodeList;
} HeadNode;

typedef struct
{
    int n_nodes;
    HeadNode *head;
} SharedGraph;

SharedGraph *init_graph(int);
Node *init_node(int);
void insert_node(SharedGraph *, int, int);

int main()
{
    key_t key = ftok(FILEPATH, KEY);
    key_t shmid = shmget(key, 1024, IPC_CREAT | 0666);

    return 0;
}

SharedGraph *init_graph(int n_nodes)
{
    SharedGraph *graph = (SharedGraph *)malloc(sizeof(SharedGraph));
    graph->n_nodes = n_nodes;
    graph->head = (HeadNode *)malloc(sizeof(HeadNode) * n_nodes);
    for (int i = 0; i < n_nodes; i++)
    {
        graph->head[i].NodeList = NULL;
    }
    return graph;
}

Node *init_node(int data)
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->data = data;
    node->next = NULL;
    return node;
}

void insert_node(SharedGraph *graph, int src, int dest)
{
    Node *n1 = init_node(src);
    Node *n2 = init_node(dest);
    Node *t = (Node *)malloc(sizeof(Node));

    if (graph->head[dest].NodeList == NULL)
        graph->head[dest].NodeList = n1;
    else
    {
        n1->next = graph->head[dest].NodeList;
        graph->head[dest].NodeList = n1;
    }

    if (graph->head[src].NodeList == NULL)
        graph->head[src].NodeList = n2;
    else
    {
        n2->next = graph->head[src].NodeList;
        graph->head[src].NodeList = n2;
    }
}