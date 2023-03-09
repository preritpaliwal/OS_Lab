// #########################################
// ## Ashwani Kumar Kamal (20CS10011)     ##
// ## Pranav Nyati (20CS30037)            ##
// ## Prerit Paliwal (20CS10046)          ##
// ## Vibhu (20CS10072)                   ##
// ## Operating Systems Laboratory        ##
// ## Assignment - 5                      ##
// #########################################

#include <iostream>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h> // to use sleep

using namespace std;

#define N_USER_SIMULATOR_THREADS 1
#define N_PUSH_UPDATE_THREADS 25
#define N_READ_POST_THREADS 10
#define N_LOCKS 2
#define N_QUEUES 2
#define N_THREADS N_USER_SIMULATOR_THREADS+N_PUSH_UPDATE_THREADS+N_READ_POST_THREADS

enum ActionType { POST, COMMENT, LIKE };

struct Action{
    int userId;
    int actionId;
    int actionType;
    time_t timeStamp;
    int recId;
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
    int chorono;  // what is this used for ? Can't find it in the code other than the init function
    int degree;
    int actionCounter;  // will need 3 counters for each action type
    ActionQueue wallQueue;
    ActionQueue feedQueue;
    int neighSize;
    int *neighbors;
    int *priority;
};

struct Graph{
    int num_edges;
    int num_nodes;
    Node *graphNodes;
};

int ITERATIONS = 15;
int threadToJoin = 0;
Graph graph;
FILE *logFile;
ActionQueue sharedQueues[N_QUEUES];
pthread_mutex_t lock[N_LOCKS];
pthread_t t_id[N_THREADS];

char graphFilePath[] = "musae_git_edges.csv";
char logFilePath[] = "sns.log";


void segFaultHandler(int sig) {
    void *array[10];
    size_t size;

    size = backtrace(array, 10);

    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

void initLogFile(){
    logFile = fopen(logFilePath, "w");
    if (NULL == logFile)
    {
        printf("log file can't be opened \n");
        exit(EXIT_FAILURE);
    }
}

void initAllLocks(){
    for(int i = 0;i<N_LOCKS;i++){
        if (pthread_mutex_init(&(lock[i]), NULL) != 0) {
            printf("\n mutex init has failed\n");
            exit(EXIT_FAILURE);
        }
    }
}

void destroyAllLocks(){
    for(int i = 0;i<N_LOCKS;i++){
        if (pthread_mutex_destroy(&(lock[i])) != 0) {
            printf("\n mutex destroy has failed\n");
            exit(EXIT_FAILURE);
        }
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
    a->recId = -1;
    return a;
}

Action *generateAction(Action *a1,Node *n){
    Action *a = (Action *)malloc(sizeof(Action));
    a->userId = a1->userId;
    a->actionId = a1->actionId;
    a->actionType = a1->actionType;
    a->timeStamp = a1->timeStamp;
    a->recId = n->number;
    return a;
}

ActionNode *generateActionNode(Action *a){
    ActionNode *an = (ActionNode *)malloc(sizeof(ActionNode));
    an->action = a;
    an->next = NULL;
    return an;
}

void initActionQueue(ActionQueue *aq){
    aq->head = NULL;
    aq->tail = NULL;
    aq->len = 0;
}

void initQueues(){
    for(int i = 0;i<N_QUEUES;i++){
        initActionQueue(&(sharedQueues[i]));
    }
}

int pushActionQueue(ActionQueue *aq, ActionNode *an){
    if(aq->head==NULL){
        aq->head = an;
    }
    if(aq->tail != NULL){
        aq->tail->next = an;    
    }
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
    n->neighSize = 5;
    n->neighbors = (int *)malloc(n->neighSize*sizeof(int));
    n->priority = NULL;
    for(int i = 0;i<n->neighSize;i++){
        n->neighbors[i] = -1;
    }
}


void initGraph(){
    graph.num_edges = 0;
    graph.num_nodes = 37700;
    graph.graphNodes = (Node *)malloc((graph.num_nodes+1)*sizeof(Node));
    if(graph.graphNodes==NULL){
        printf("Error!! could not allocate new memory\n");
        exit(EXIT_FAILURE);
    }
    for(int i = 0;i<graph.num_nodes+1;i++){
        initNode(&(graph.graphNodes[i]),i);
    }
}

void reallocateNeighbor(Node *n){
    int tmp[n->neighSize];
    for(int i = 0;i<n->neighSize;i++){
        tmp[i] = n->neighbors[i];
    }
    n->neighbors = (int *)realloc(n->neighbors,2*(n->neighSize)*(sizeof(int)));
    if(n->neighbors==NULL){
        printf("Error!! could not allocate new memory\n");
        exit(EXIT_FAILURE);
    }
    for(int i = 0;i<n->neighSize;i++){
        n->neighbors[i] = tmp[i];
    }
    n->neighSize *=2;
}

void addNeighbor(Node *n1,Node *n2){
    if(n1->degree+1 == n1->neighSize){
        reallocateNeighbor(n1);
    }
    if(n2->degree+1 == n2->neighSize){
        reallocateNeighbor(n2);
    }
    n1->neighbors[(n1->degree)++] = n2->number;
    n2->neighbors[(n2->degree)++] = n1->number;
}

void addEdge(int n1,int n2){
    addNeighbor(&(graph.graphNodes[n1]),&(graph.graphNodes[n2]));
    graph.num_edges++;
}

void readGraph(){
    FILE *graphFile = fopen(graphFilePath, "r");
    if (graphFile == NULL){
        perror("Error while opening the graph file.\n");
        exit(EXIT_FAILURE);
    }
    char line[100];
    int idx = 0;
    while (fgets(line, sizeof(line), graphFile) != NULL){
        idx++;
        // cout<<idx<<endl;
        if(idx==1){
            continue;
        }
        int node1, node2;
        sscanf(line, "%d,%d", &node1, &node2);
        // cout<<line<<" "<<node1<<" "<<node2<<endl;
        addEdge(node1,node2);
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
        curNode->priority = (int *)malloc(curNode->degree*(sizeof(int)));
        for(int j = 0;j<curNode->degree;j++){
            curNode->priority[j] = calcPriorityUtil(curNode,&(graph.graphNodes[curNode->neighbors[j]]));
        }
    }
}

void *userSimulator(void *args){
    int actionProportionalityConstant = 2;
    // cout<<"called userSimulator"<<endl;
    while(ITERATIONS--){
        // select 100 random nodes;
        printf("\n\n####ITERATION: %d####\n\n",ITERATIONS);
        fprintf(logFile,"\n\n####ITERATION: %d####\n\n",ITERATIONS);
        srand(time(NULL));
        for(int _ = 0;_<100;_++){
            int nodeNum = getRandomNumber();
            Node *curNode = &(graph.graphNodes[nodeNum]);
            
            int NumberOfActions = 1+actionProportionalityConstant*log2(curNode->degree);
            printf("Node Number: %d\nNum of Action: %d, Degree: %d\n\n",nodeNum,NumberOfActions,curNode->degree);
            fprintf(logFile,"Node Number: %d\nNum of Action: %d, Degree: %d\n\n",nodeNum,NumberOfActions,curNode->degree);
            
            for(int __ = 0;__<NumberOfActions;__++){
                Action *action = generateAction(curNode);
                // pushActionQueue(&(curNode->wallQueue),action);
                pthread_mutex_lock(&(lock[0]));
                printf("[User Simulator Thread]Pushed Action:- userID:%d,actionId,%d,actionType:%d,timeStamp:%ld\n",action->userId,action->actionId,action->actionType,action->timeStamp);
                fprintf(logFile,"[User Simulator Thread]Pushed Action:- userID:%d,actionId,%d,actionType:%d,timeStamp:%ld\n",action->userId,action->actionId,action->actionType,action->timeStamp);
                pushActionQueue(&(sharedQueues[0]),action);
                pthread_mutex_unlock(&(lock[0]));
            }
        }
        // sleep for 2 seconds
        sleep(2);
    }
    printf("\n\n[User Simulator]Done taking actions, Byee!!\n\n");
    fprintf(logFile,"\n\n[User Simulator]Done taking actions, Byee!!\n\n");
    return 0;
}

void *pushUpdate(void *args){
    // cout<<"called pushUpdate"<<endl;
    while(true){
        pthread_mutex_lock(&(lock[0]));
        if((sharedQueues[0]).len==0){
            pthread_mutex_unlock(&(lock[0]));
            if(threadToJoin>=N_USER_SIMULATOR_THREADS ){
                break;
            }
            continue;
        }
        ActionNode *curActionNode = (sharedQueues[0]).head;
        Action *curAction = curActionNode->action;
        popActionQueue(&(sharedQueues[0]));
        printf("\t[Push Update Thread]Popped Action:- userID:%d,actionId,%d,actionType:%d,timeStamp:%ld,recID:%d\n",curAction->userId,curAction->actionId,curAction->actionType,curAction->timeStamp,curAction->recId);
        fprintf(logFile,"\t[Push Update Thread]Popped Action:- userID:%d,actionId,%d,actionType:%d,timeStamp:%ld,recID:%d\n",curAction->userId,curAction->actionId,curAction->actionType,curAction->timeStamp,curAction->recId);
        pthread_mutex_unlock(&(lock[0]));
        Node *curNode = &(graph.graphNodes[curAction->userId]);
        for(int i = 0;i<curNode->degree;i++){
            Node *curNeigh = &(graph.graphNodes[curNode->neighbors[i]]);
            Action *newAction = generateAction(curAction,curNeigh);
            // pushActionQueue(&(curNeigh->feedQueue),newAction);
            pthread_mutex_lock(&(lock[1]));
            printf("\t[Push Update Thread]Pushed Action:- userID:%d,actionId,%d,actionType:%d,timeStamp:%ld,recID:%d\n",newAction->userId,newAction->actionId,newAction->actionType,newAction->timeStamp,newAction->recId);
            fprintf(logFile,"\t[Push Update Thread]Pushed Action:- userID:%d,actionId,%d,actionType:%d,timeStamp:%ld,recID:%d\n",newAction->userId,newAction->actionId,newAction->actionType,newAction->timeStamp,newAction->recId);
            pushActionQueue(&(sharedQueues[1]),newAction);
            pthread_mutex_unlock(&(lock[1]));
        }
        free(curAction);
        free(curActionNode);
    }
    printf("\n\n[Push Update]Done taking actions, Byee!!\n\n");
    fprintf(logFile,"\n\n[Push Update]Done taking actions, Byee!!\n\n");
    return 0;
}

void *readPost(void *args){
    // cout<<"Called readpost"<<endl;
    while(true){
        pthread_mutex_lock(&(lock[1]));
        if(sharedQueues[1].len==0){
            pthread_mutex_unlock(&(lock[1]));
            if(threadToJoin>=N_USER_SIMULATOR_THREADS+N_PUSH_UPDATE_THREADS){
                break;
            }
            continue;
        }
        ActionNode *curActionNode = (sharedQueues[1]).head;
        Action *curAction = curActionNode->action;
        popActionQueue(&(sharedQueues[1]));
        printf("\t\t[Read Post Thread]Popped Action:- userID:%d,actionId,%d,actionType:%d,timeStamp:%ld,recID:%d\n",curAction->userId,curAction->actionId,curAction->actionType,curAction->timeStamp,curAction->recId);
        fprintf(logFile,"\t\t[Read Post Thread]Popped Action:- userID:%d,actionId,%d,actionType:%d,timeStamp:%ld,recID:%d\n",curAction->userId,curAction->actionId,curAction->actionType,curAction->timeStamp,curAction->recId);
        pthread_mutex_unlock(&(lock[1]));
        printf("I {node number = %d} read action number %d of type %d posted by user %d at time %ld\n",curAction->recId,curAction->actionId,curAction->actionType,curAction->userId,curAction->timeStamp);
        fprintf(logFile,"I {node number = %d} read action number %d of type %d posted by user %d at time %ld\n",curAction->recId,curAction->actionId,curAction->actionType,curAction->userId,curAction->timeStamp);
        free(curAction);
        free(curActionNode);
    }
    printf("\n\n[Read Post]Done taking actions, Byee!!\n\n");
    fprintf(logFile,"\n\n[Read Post]Done taking actions, Byee!!\n\n");
    return 0;
}


void runThreads(){
    for(int i = 0;i<N_THREADS;i++){
        fprintf(logFile,"Creating Thread Number: %d\n",i);
        printf("Creating Thread Number: %d\n",i);
        if(i<N_USER_SIMULATOR_THREADS){
            if(pthread_create(&(t_id[i]),NULL,userSimulator,NULL)!=0){
                printf("\n pthread create has failed\n");
                exit(EXIT_FAILURE);
            }
        }
        else if(i<N_USER_SIMULATOR_THREADS+N_PUSH_UPDATE_THREADS){
            if(pthread_create(&(t_id[i]),NULL,pushUpdate,NULL)!=0){
                printf("\n pthread create has failed\n");
                exit(EXIT_FAILURE);
            }
        }
        else{
            if(pthread_create(&(t_id[i]),NULL,readPost,NULL)!=0){
                printf("\n pthread create has failed\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void joinThreads(){
    for(int i = 0;i<N_THREADS;i++){
        pthread_join(t_id[i],NULL);
        threadToJoin++;
    }
}

void getDegreeAndNeighbor(){
    int deg = 0;
    for(int i = 0;i<graph.num_nodes;i++){
        deg = max(deg,graph.graphNodes[i].degree);
        fprintf(logFile,"%d's degree : %d\nNeighbors are: ",i,graph.graphNodes[i].degree);
        for(int j = 0;j<graph.graphNodes[i].degree;j++){
            fprintf(logFile,"%d,",graph.graphNodes[i].neighbors[j]);
        }
        fprintf(logFile,"\nPriority : ");
        for(int j = 0;j<graph.graphNodes[i].degree;j++){
            fprintf(logFile,"%d,",graph.graphNodes[i].priority[j]);
        }
        fprintf(logFile,"\n");
    }
    fprintf(logFile,"max degree: %d\n",deg);


}

int main()
{
    signal(SIGSEGV, segFaultHandler);   // install our handler

    initGraph();
    initLogFile();
    initQueues();
    readGraph();
    calcPriority();
    // getDegreeAndNeighbor();

    printf("Number of Edges: %d\n", graph.num_edges);
    printf("Number of Nodes: %d\n", graph.num_nodes);

    initAllLocks();
    runThreads();
    joinThreads();
    destroyAllLocks();

    fclose(logFile);
    return 0;
}