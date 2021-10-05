#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

int main() {

    // int in_def = dup(0);
    // int out_def = dup(1);

    while (true) {
        // dup2(in_def, 0);
        // dup2(out_def, 1);

        cout << "My Shell$ ";
        string inputline;
        getline (cin, inputline);
        if (inputline == string("exit")) {
            cout << "Bye!! End of shell" << endl;
            break;
        }
        int pid = fork();
        if(pid == 0) {
            char* args [] = {(char *) inputline.c_str(), NULL};
            execvp (args [0], args);
        }
    }


    return 0;
}