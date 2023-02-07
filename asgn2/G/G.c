#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include <time.h>
#include <unistd.h>

int loop  = 1;
void handle_parent(int sig)
{
	printf("PARENT: Caught signal %d, won't exit\n", sig);
	return;
}

void handle_child(int sig)
{
	printf("CHILD: Caught signal %d\n", sig);
	loop=0;
	exit(sig);
}

int main()
{
	int status;
	int ret_pid;
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
			ret_pid = waitpid(pid, &status, WNOHANG);
			if(ret_pid == pid)
			{
				printf("PARENT: Child exited with status %d\n", status);
				break;
			}
			printf("inside PARENT while\n");
			sleep(1);
		}
    } 

	printf("outside while\n");
	return 0;
}


