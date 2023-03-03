#include <iostream>
#include <pthread.h>
#include <time.h>
#include <math.h>
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
    int userId;
    int actionId;
    int actionType;
    time_t timeStamp;
};

struct ActionNode{
    Action *action;
    ActionNode* next;
};

struct ActionQueue{
    ActionNode *head;
    ActionNode *tail;
    int len;
};

struct Node{
    int number;
    int chorono;
    int degree;
    int actionCounter;
    ActionQueue wallQueue;
    ActionQueue feedQueue;
    int *neighbors;
    /*
    TO DO: Precompute this based on the number of common neighbors.
    */
    // int *priority 
};

struct Graph{
    int num_edges;
    int num_nodes;
    Node *graphNodes;
};

Graph graph;
FILE *logFile;
ActionQueue newActions;
char graphFilePath[] = "musae_git_edges.csv";
char logFilePath[] = "sns.log";

void initGraph(){
    graph.graphNodes = NULL;
    graph.num_edges = 0;
    graph.num_nodes = 0;
}

void initLogFile(){
    logFile = fopen(logFilePath, "w");
    if (NULL == logFile)
    {
        printf("log file can't be opened \n");
        exit(EXIT_FAILURE);
    }
}

int getRandomNumber(int upper = graph.num_nodes,int lower = 0){
    return ((rand()%(upper-lower))+ lower);
}

Action *generateAction(Node *n){
    Action *a = (Action *)malloc(sizeof(Action));
    a->userId = n->number;
    a->actionId = ++(n->actionCounter);
    a->actionType = getRandomNumber(3);
    a->timeStamp = time(NULL);
    return a;
}

void initActionQueue(ActionQueue *aq){
    aq->head = NULL;
    aq->tail = NULL;
    aq->len = 0;
}

ActionNode *generateActionNode(Action *a){
    ActionNode *an = (ActionNode *)malloc(sizeof(ActionNode));
    an->action = a;
    an->next = NULL;
    return an;
}

int pushActionQueue(ActionQueue *aq, ActionNode *an){
    an->next = aq->tail;
    aq->tail = an;
    return ++(aq->len);
}

int pushActionQueue(ActionQueue *aq, Action *a){
    return pushActionQueue(aq,generateActionNode(a));
}

int popActionQueue(ActionQueue *aq){
    aq->head = aq->head->next;
    return --(aq->len);
}

void initNode(Node *n,int num){
    n->number = num;
    n->chorono = 1;
    n->degree = 0;
    n->actionCounter = 0;
    initActionQueue(&(n->wallQueue));
    initActionQueue(&(n->feedQueue));
    n->neighbors = NULL;
}

void addNeighbor(Node *n1,Node *n2){
    n1->degree++;
    n1->neighbors = (int *)realloc(n1->neighbors,n1->degree*(sizeof(int)));
    if(n1->neighbors==NULL){
        printf("Error!! could not allocate new memory\n");
        exit(EXIT_FAILURE);
    }
    n2->degree++;
    n2->neighbors = (int *)realloc(n2->neighbors,n2->degree*(sizeof(int)));
    if(n2->neighbors==NULL){
        printf("Error!! could not allocate new memory\n");
        exit(EXIT_FAILURE);
    }
    n1->neighbors[n1->degree -1] = n2->number;
    n2->neighbors[n2->degree -1] = n1->number;
}

void addEdge(Graph *graph,int n1,int n2){
    int posmax = max(n1,n2);
    if(posmax>=graph->num_nodes){
        // cout<<"num of nodes"<<graph->num_nodes<<endl;
        graph->graphNodes = (Node *)realloc(graph->graphNodes,(posmax+1)*sizeof(Node));
        if(graph->graphNodes==NULL){
            printf("Error!! could not allocate new memory\n");
            exit(EXIT_FAILURE);
        }
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

void readGraph(){
    // read the contents of the input file
    FILE *graphFile = fopen(graphFilePath, "r");
    if (graphFile == NULL){
        perror("Error while opening the graph file.\n");
        exit(EXIT_FAILURE);
    }


    char line[100];
    int idx = 0;
    while (fgets(line, sizeof(line), graphFile) != NULL){
        idx++;
        if(idx==1){
            continue;
        }
        int node1, node2;
        sscanf(line, "%d,%d", &node1, &node2);
        addEdge(&graph,node1,node2);
    }
    fclose(graphFile);
}

void *userSimulator(void *args){
    int actionproportionalityConstant = 2;
    while(true){
        // select 100 random nodes;
        srand(time(0));
        for(int _ = 0;_<100;_++){
            int nodeNum = getRandomNumber();
            Node curNode = graph.graphNodes[nodeNum];
            
            int NumberOfActions = 1+actionproportionalityConstant*log2(curNode.degree);
            printf("Node Number: %d\nNum of Action: %d, Degree: %d\n\n",nodeNum,NumberOfActions,curNode.degree);
            fprintf(logFile,"Node Number: %d\nNum of Action: %d, Degree: %d\n\n",nodeNum,NumberOfActions,curNode.degree);
            
            for(int __ = 0;__<NumberOfActions;__++){
                Action *action = generateAction(&curNode);
                printf("Action: \nuserID:%d,actionId,%d,actionType:%d,timeStamp:%ld\n",action->userId,action->actionId,action->actionType,action->timeStamp);
                fprintf(logFile,"Action: \nuserID:%d,actionId,%d,actionType:%d,timeStamp:%ld\n",action->userId,action->actionId,action->actionType,action->timeStamp);
                pushActionQueue(&(curNode.wallQueue),action);
                pushActionQueue(&newActions,action);
                printf("pushed in both queues\n");
                fprintf(logFile,"pushed in both queues\n");
            }
        }
        // sleep for 2 seconds
        sleep(2);
    }
}

void pushUpdate(){
    return;
}

void readPost(){
    return;
}



int main()
{
    initGraph();
    initLogFile();
    initActionQueue(&newActions);
    readGraph();

    printf("Number of Edges: %d\n", graph.num_edges);
    printf("Number of Nodes: %d\n", graph.num_nodes);

    pthread_t t_id;
    printf("Before creating thread\n");
    pthread_create(&t_id,NULL,userSimulator,NULL);
    pthread_join(t_id,NULL);
    printf("Thread created\n");

    fclose(logFile);
    return 0;
}