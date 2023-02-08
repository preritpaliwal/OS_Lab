#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <vector>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Usage: " << argv[0] << " <filepath>" << endl;
        return 1;
    }
    int fd[2];
    pipe(fd);
    pid_t p = fork();

    if (p > 0)
    {
        // parent process
        wait(NULL);
        close(fd[1]);
        dup(fd[0]);

        // read recieved root
        int *pids = (int *)malloc(100 * sizeof(int));
        for (int i = 0; i < 100; i++)
            pids[i] = -1;

        read(fd[0], pids, sizeof(pids));

        cout << "processes locking file:"
             << "\n";
        for (int i = 0; pids[i] > 0; i++)
        {
            cout << pids[i] << endl;
        }
    }
    else if (p == 0)
    {
        close(fd[0]);
        dup(fd[1]);

        int *pids = (int *)malloc(100 * sizeof(int));
        }
    else
    {
        perror("fork error");
    }
    return 0;
}