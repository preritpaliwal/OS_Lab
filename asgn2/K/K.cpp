#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/types.h>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <pid>" << endl;
        return 1;
    }

    string pid = argv[1];
    string ppid;

    // TOOD: read the parent pid of the process with pid from /proc
    // Write algorithm to find root ppid

    // print the pid of first parent of the process
    cout << "Parent of " << pid << " is " << ppid << endl;

    return 0;
}