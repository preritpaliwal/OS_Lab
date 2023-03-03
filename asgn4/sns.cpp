#include <iostream>
#include <pthread.h>
#include <unistd.h> // to use sleep
using namespace std;

// void *t1(void *vargp){
//     printf("thread started\n");
//     sleep(1);
//     printf("done with thread\n");
//     return NULL;
// }

int main()
{
    // pthread_t t_id;
    // printf("Before creating thread\n");
    // pthread_create(&t_id,NULL,t1,NULL);
    // pthread_join(t_id,NULL);
    // printf("Thread created\n");

    FILE *ptr;
    char str[50];
    ptr = fopen("musae_git_edges.csv", "a+");

    if (NULL == ptr)
    {
        printf("file can't be opened \n");
    }
    printf("content of this file are \n");

    int i = -1;
    while (fgets(str, 50, ptr) != NULL)
    {
        printf("%s", str);
        i++;
    }
    cout<<i<<endl;

    fclose(ptr);
    return 0;
}