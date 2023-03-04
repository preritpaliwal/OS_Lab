#include <iostream>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h> // to use sleep
using namespace std;

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
    int *priority;
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
    if(aq->head==NULL){
        aq->head = an;
    }
    an->next = aq->tail;
    aq->tail = an;
    return ++(aq->len);
}

int pushActionQueue(ActionQueue *aq, Action *a){
    return pushActionQueue(aq,generateActionNode(a));
}

int popActionQueue(ActionQueue *aq){
    if(aq->head==NULL){
        return aq->len;
    }
    aq->head = aq->head->next;
    if(aq->head==NULL){
        aq->tail = NULL;
    }
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
    n->priority = NULL;
}

void addNeighbor(Node *n1,Node *n2){
    n1->degree++;
    n1->neighbors = (int *)realloc(n1->neighbors,n1->degree*(sizeof(int)));
    if(n1->neighbors==NULL){
        printf("Error!! could not allocate new memory\n");
        exit(EXIT_FAILURE);
    }
    n1->priority = (int *)realloc(n1->priority,n1->degree*(sizeof(int)));
    if(n1->priority==NULL){
        printf("Error!! could not allocate new memory\n");
        exit(EXIT_FAILURE);
    }
    for(int i = 0;i<n1->degree;i++){
        n1->priority[i] = -1;
    }
    n2->degree++;
    n2->neighbors = (int *)realloc(n2->neighbors,n2->degree*(sizeof(int)));
    if(n2->neighbors==NULL){
        printf("Error!! could not allocate new memory\n");
        exit(EXIT_FAILURE);
    }
    n2->priority = (int *)realloc(n2->priority,n2->degree*(sizeof(int)));
    if(n2->priority==NULL){
        printf("Error!! could not allocate new memory\n");
        exit(EXIT_FAILURE);
    }
    for(int i = 0;i<n2->degree;i++){
        n2->priority[i] = -1;
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

int calcPriorityUtil(Node *n1,Node *n2){
    int common = 0;
    for(int i = 0;i<n1->degree;i++){
        for(int j = 0;j<n2->degree;j++){
            if(n1->neighbors[i] == n2->neighbors[j]){
                common++;
            }
        }
    }
    return common;
}

void calcPriority(){
    for(int i = 0;i<graph.num_nodes;i++){
        Node *curNode = &(graph.graphNodes[i]);
        for(int j = 0;j<curNode->degree;j++){
            if(curNode->priority[j]==-1){
                curNode->priority[j] = calcPriorityUtil(curNode,&(graph.graphNodes[curNode->neighbors[j]]));
            }
        }
    }
}

void *userSimulator(void *args){
    int actionproportionalityConstant = 2;
    while(true){
        // select 100 random nodes;
        srand(time(NULL));
        for(int _ = 0;_<100;_++){
            cout<<"sampling node i = "<<_<<endl;
            int nodeNum = getRandomNumber();
            Node *curNode = &(graph.graphNodes[nodeNum]);
            
            int NumberOfActions = 1+actionproportionalityConstant*log2(curNode->degree);
            // printf("Node Number: %d\nNum of Action: %d, Degree: %d\n\n",nodeNum,NumberOfActions,curNode->degree);
            fprintf(logFile,"Node Number: %d\nNum of Action: %d, Degree: %d\n\n",nodeNum,NumberOfActions,curNode->degree);
            
            for(int __ = 0;__<NumberOfActions;__++){
                Action *action = generateAction(curNode);
                // printf("Action: \nuserID:%d,actionId,%d,actionType:%d,timeStamp:%ld\n",action->userId,action->actionId,action->actionType,action->timeStamp);
                fprintf(logFile,"Action: \nuserID:%d,actionId,%d,actionType:%d,timeStamp:%ld\n",action->userId,action->actionId,action->actionType,action->timeStamp);
                pushActionQueue(&(curNode->wallQueue),action);
                pushActionQueue(&newActions,action);
                // printf("pushed in both queues\n");
                // fprintf(logFile,"pushed in both queues\n");
            }
        }
        // sleep for 2 seconds
        sleep(2);
    }
}

void *pushUpdate(void *args){
    cout<<"Running pushUpdate"<<endl;
    while(true){
        if(newActions.len==0){
            continue;
        }
        cout<<"Finally have something to do"<<endl;
        // PUT A LOCK ON THIS!!
        Action *curAction = newActions.head->action;
        cout<<"##### selected action"<<endl;
        popActionQueue(&newActions);
        cout<<"##### POPPED out the action"<<endl;

        Node *curNode = &(graph.graphNodes[curAction->userId]);
        for(int i = 0;i<curNode->degree;i++){
            cout<<"#####ITERATING ON NEighbors i = "<<i<<endl;
            Node *curNeigh = &(graph.graphNodes[curNode->neighbors[i]]);
            // PUT A LOCK ON THIS ASWELL
            pushActionQueue(&(curNeigh->feedQueue),curAction);
            cout<<"\n\n#####Pushing in feedqueue of all neighbors##########\n\n"<<endl;
        }
    }
    return 0;
}

void *readPost(void *args){
    return 0;;
}

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

int main()
{
    signal(SIGSEGV, handler);   // install our handler

    initGraph();
    initLogFile();
    initActionQueue(&newActions);
    readGraph();
    calcPriority();

    printf("Number of Edges: %d\n", graph.num_edges);
    printf("Number of Nodes: %d\n", graph.num_nodes);

    pthread_t t_id[3];
    for(int i = 0;i<3;i++){
        fprintf(logFile,"Creating Thread Number: %d",i);
        printf("Creating Thread Number: %d",i);
        if(i==0){
            pthread_create(&(t_id[i]),NULL,userSimulator,NULL);
        }
        else if(i<2){
            pthread_create(&(t_id[i]),NULL,pushUpdate,NULL);
        }
        else{
            pthread_create(&(t_id[i]),NULL,readPost,NULL);
        }
    }
    for(int i = 0;i<36;i++){
        pthread_join(t_id[i],NULL);
    }
    fclose(logFile);
    return 0;
}