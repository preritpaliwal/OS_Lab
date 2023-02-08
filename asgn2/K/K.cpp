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

using namespace std;

int getGroup(int pid)
{
    char procname[32];
    FILE *fp;

    snprintf(procname, sizeof(procname), "/proc/%u/stat", pid);
    fp = fopen(procname, "r");

    if (fp == NULL)
    {
        cerr << "could not open the stat file " << procname << endl;
        return 0;
    }

    char c;
    int group_f = 0;
    int gpid = 0;

    while ((c = fgetc(fp)) != EOF)
    {
        if (c == ' ')
            group_f++;

        if (group_f == 4)
        {
            while ((c = fgetc(fp)) != EOF)
            {
                if (c == ' ')
                    break;
                gpid = gpid * 10 + (int)(c - '0');
            }
            break;
        }
    }
    fclose(fp);
    return gpid;
}

int getParent(int pid, char *process)
{
    char procname[32];
    FILE *fp;

    snprintf(procname, sizeof(procname), "/proc/%u/stat", pid);
    cout << procname << endl;
    fp = fopen(procname, "r");

    if (fp == NULL)
    {
        cerr << "could not open the stat file " << procname << endl;
        return 0;
    }

    char c;
    int parent_pid_f = 0;
    int process_f = 0;
    int ppid = 0;

    while ((c = fgetc(fp)) != EOF)
    {
        if (c == ' ')
        {
            parent_pid_f++;
            process_f++;
        }
        if (process_f == 1)
        {
            if (c == '(' || c == ' ')
                continue;
            if (c == ')')
            {
                process_f++;
                continue;
            }
            *process++ = c;
        }
        if (parent_pid_f == 3)
        {
            while ((c = fgetc(fp)) != EOF)
            {
                if (c == ' ')
                    break;
                ppid = ppid * 10 + (int)(c - '0');
            }
            break;
        }
    }
    fclose(fp);
    return ppid;
}

pair<int, int> getRoot(int pid)
{
    char *process = (char *)malloc(32 * sizeof(char));
    char *parentProcess = (char *)malloc(32 * sizeof(char));

    int parent = getParent(pid, process);
    int grandParent = getParent(parent, parentProcess);

    // Base case
    if (parent <= 1 || strcmp(process, parentProcess) != 0)
    {
        try
        {
            free(process);
            free(parentProcess);

            // orphaned child: adopted by init
            int gpid = getGroup(pid);
            return {pid, gpid};
        }
        catch (...)
        {
            cerr << "error killing process " << pid << endl;
            return {0, 0};
        }
    }

    try
    {
        free(process);
        free(parentProcess);
        // kill(pid, SIGSEGV);
        return getRoot(parent);
    }
    catch (...)
    {
        cerr << "error killing process " << pid << endl;
        return {0, 0};
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Usage: " << argv[0] << " <pid>" << endl;
        return 1;
    }
    int pid;
    try
    {
        pid = stoi(argv[1]);
    }
    catch (...)
    {
        cerr << "invalid pid: " << argv[1] << endl;
        exit(EXIT_FAILURE);
    }

    // create pipe for communication in parent-child
    int fd[2];
    pipe(fd);

    // fork child
    pid_t p = fork();

    if (p > 0)
    {
        // parent process
        wait(NULL);
        close(fd[1]);
        dup(fd[0]);

        // read recieved root
        int root;
        read(fd[0], &root, sizeof(root));

        cout << "Root of " << pid << " is " << root << endl;
    }
    else if (p == 0)
    {
        close(fd[0]);
        dup(fd[1]);

        char *process = (char *)malloc(32 * sizeof(char));
        // get root of bug
        pair<int, int> p;
        p = getRoot(pid);

        cout << "killing -" << p.second << endl;
        kill(-1 * p.second, SIGSEGV);
        // send back to parent process
        write(fd[1], &p.second, sizeof(p.first));
    }
    else
    {
        perror("fork error");
    }

    return 0;
}