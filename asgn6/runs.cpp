#include "goodmalloc.h"

#include <time.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

int main(){

    for (int i = 0; i < 100; i++){
        if (fork() == 0){
            int ret = execlp("./test", "./test", NULL);
            if (ret == -1){
                cout << "Error in execlp" << endl;
            }
        }

        else {
            int status;
            wait(&status);
        }
    }


}