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
    // char* cmd = "wc";
    // char* args[] = {"wc", "-w", "-l", "--bytes", infile , NULL};

    char* cmd = "ls";
    char* args[] = {"ls", "-l", NULL};

    int fd = open(outfile, O_WRONLY | O_APPEND);
    dup2(fd,1);

    execvp(cmd,args); // args

    // execv() //stdin

    close(fd);

    // ./a.out < infile.txt
    // ./a.out infile.txt

}