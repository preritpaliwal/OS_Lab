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
#include <fcntl.h>

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

    if (fcntl(fd[0], F_SETFL, O_NONBLOCK) < 0)
        exit(2);

    pid_t p = fork();

    if (p > 0)
    {
        // parent process
        wait(NULL);
        close(fd[1]);
        dup(fd[0]);

        // read recieved pids
        int *pids = (int *)malloc(100 * sizeof(int));
        for (int i = 0; i < 100; i++)
            pids[i] = -1;

        char buffer[1024];

        while (read(fd[0], buffer, sizeof(buffer)) != 0)
        {
            char *token = strtok(buffer, " ");
            int i = 0;
            while (token != NULL)
            {
                pids[i] = atoi(token);
                token = strtok(NULL, " ");
                i++;
            }
        }

        // print pids
        cout << "processes locking file:"
             << "\n";
        for (int i = 0; pids[i] > 0; i++)
        {
            cout << pids[i] << endl;
        }
        cout << "kill? (y/n): "
             << "\n";
        char c;
        cin >> c;
        if (c == 'y')
        {
            for (int i = 0; pids[i] > 0; i++)
            {
                kill(pids[i], SIGKILL);
            }
            remove(argv[1]);
        }
    }
    else if (p == 0)
    {
        close(1);
        close(fd[0]);
        dup(fd[1]);

        // get pids locking on the file
        int *pids = (int *)malloc(100 * sizeof(int));
        if (execlp("fuser", "fuser", argv[1], (char *)NULL) < 0)
        {
            perror("execlp error");
        }
    }
    else
    {
        perror("fork error");
    }
    return 0;
}