#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

int main()
{

    // Infile and Outfile are extracted from parser as arguements after < and > respectively
    char* infile = "infile.txt";
    char* outfile = "outfile.txt";

    // Cmd and Args are extracted from parser
    char* args[] = {"wc", "-w", "-l", "--bytes", infile , NULL};

    char* cmd = "wc";

    int fd = open(outfile, O_WRONLY | O_APPEND);
    dup2(fd,1);
    close(fd);

    execvp(cmd,args);

}