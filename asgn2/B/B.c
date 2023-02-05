#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

int main()
{

    // Infile and Outfile are extracted from parser as arguements after < and > respectively
    char *infile = "infile.txt";

    int fd = open(infile, O_RDONLY);
    close(0);
    dup(fd);

    char *cmd = "./a.out";
    char *args[] = {"./a.out", NULL};

    execv(cmd, args);

    close(fd);
}