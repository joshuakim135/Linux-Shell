#include <iostream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std;
string trim (string input) {
    int i = 0;
    while (i < input.size() && input[i] == ' ')
        i++;
    if (i < input.size())
        input = input.substr(i);
    else {
        return "";
    }

    i = input.size() - 1;
    while (i >= 0 && input[i] == ' ')
        i--;
    if (i >= 0)
        input = input.substr (0, i+1);
    else
        return "";
    
    return input;
}

vector<string> split(string &str, char delim = ' ') {
    vector<string> out;
    if (delim == '|') {
        std::stringstream ss(str);
        string item;

        while(getline(ss, item, delim)) {
            out.push_back(trim(item));
        }
    } else {
        std::istringstream iss(str);
        string s;
        while (iss >> std::quoted(s)) {
            out.push_back(trim(s));
        }
    }
    return out;
}

char** vec_to_char_array (vector<string>& parts) {
    char** result = new char * [parts.size() + 1]; // add 1 for the NULL pointer at the end
    for (int i = 0; i < parts.size(); i++) {
        result[i] = (char*) parts[i].c_str();
    }
    result [parts.size()] = NULL;
    return result;
}

int main() {
    /*
    string str = "echo \"Hello World\" | echo \"Hi\"";
    cout << "str: " << str << endl;
    vector<string> splitted = split(str, '|');
    for (string word : splitted) {
        cout << word << endl;
    }

    cout << splitted[0] << " => " << endl;
    vector<string> c = split(splitted[0]);
    for (string c_str : c) {
        cout << " word: " << c_str << endl;
    }

    exit(0);
    */

    int in_def = dup(0);
    int out_def = dup(1);
    
    vector<int> bgs;
    string previousCmd;

    while (true) {
        // reset file descriptor
        dup2(in_def, 0);
        dup2(out_def, 1);

        // clean up background process
        for (int i = 0; i < bgs.size(); i++) {
           if (waitpid(bgs[i], 0, WNOHANG) == bgs[i]) {
               cout << "Process: " << bgs[i] << " ended" << endl;
               bgs.erase (bgs.begin() + i);
               i--;
           }
        }

        // shell prompt
        string userName = getlogin();
        cout << userName << "$ ";
        string inputline;
        getline(cin, inputline);

        // check if exit
        if (inputline == string("exit")) {
            cout << "Bye!1 End of shell" << endl;
            break;
        }

        bool bg = false;
        inputline = trim(inputline);

        // if inputline has & at end
        if (inputline[inputline.size()-1] == '&') {
            bg = true;
            inputline = inputline.substr (0, inputline.size()-1);
        }
        
        // check if it's a 'cd' command
        if (inputline.length() > 2) {
            if ((inputline[0] == 'c') && (inputline[1] == 'd')) {
                // if cd command, execute it here
                // check if 'cd -' command
                if (inputline.length() > 3) {
                    if (inputline[3] == '-') {
                        // get current working directory
                        char buffer[260];
                        char *path = getcwd(buffer, 260);
                        string paththing = path;
                        if (path) {
                            // get current dir
                            string CurrentPath;
                            CurrentPath = path;
                            // change current dir to prev dir
                            chdir(previousCmd.c_str());

                            // change previous dir to old current
                            previousCmd = CurrentPath;
                            continue;
                        }
                    }
                }
                string dir_str = "";
                for (int i = 2; i < inputline.size(); i++) {
                    dir_str += inputline[i];
                }
                dir_str = dir_str.substr(1, dir_str.length()-1);

                char buffer2[260];
                char *path2 = getcwd(buffer2, 260);
                string CurrentPath2;
                CurrentPath2 = path2;
                int rx = chdir(dir_str.c_str());
                if (chdir < 0) {
                    cout << "chdir() failed" << endl;
                } else {
                    previousCmd = CurrentPath2;
                }
                continue;
            } 
        }

        // check for '<' and '>'
        bool in = false, out = false;
        if (inputline.find("<") != string::npos) {
            in = true;
        }
        if (inputline.find(">") != string::npos) {
            out = true;
        }

        // split by '|'
        vector<string> c = split(inputline, '|');

        // fork + exec starts here
        for (int i = 0; i < c.size(); i++) {
            int fd[2];
            pipe(fd);
            int pid = fork();
            if (pid == -1) {
                perror("fork");
            } else if (pid == 0) { // if child
                vector<string> parts = split(c[i]);

                char** args = vec_to_char_array(parts);
                execvp (args[0], args);
            } else { // if parent
                if (!bg) {
                    waitpid(pid, 0, 0); // wait for child process
                } else {
                    bgs.push_back(pid);
                }
            }
        }
    }

    return 0; 
}