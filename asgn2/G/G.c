#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include <time.h>
#include <unistd.h>

int loop  = 1;
void handle_parent(int sig)
{
	return;
}

void handle_child(int sig)
{
	printf("Caught signal %d\n", sig);
	loop=0;
	exit(sig);
}

int main()
{
	int pid=fork();
	if(pid!=0)	
		signal(SIGINT, handle_parent);
    else
		signal(SIGINT, handle_child);
	if(pid==0){
		while(1)
	    {
			printf("inside CHILD while\n");
			sleep(1);
		}
    } 
	else{
		while(1)
	    {
			printf("inside PARENT while\n");
			sleep(1);
		}
    } 

	printf("outside while\n");
	return 0;
}
