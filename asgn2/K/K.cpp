#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

using namespace std;

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

int getRoot(int pid)
{
    char *process = (char *)malloc(32 * sizeof(char));
    char *parentProcess = (char *)malloc(32 * sizeof(char));

    int parent = getParent(pid, process);
    int grandParent = getParent(parent, parentProcess);

    if (parent == 1 || strcmp(process, parentProcess) != 0)
        return pid;

    free(process);
    free(parentProcess);

    return getRoot(parent);
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
    char *process = (char *)malloc(32 * sizeof(char));
    int parent = getParent(pid, process);
    cout << "Parent of " << pid << " (" << process << ") is " << parent << endl;

    int root = getRoot(pid);
    cout << "Root of " << pid << " is " << root << endl;

    // TOOD: read the parent pid of the process with pid from /proc
    // Write algorithm to find root ppi

    // print the pid of first parent of the process
    return 0;
}