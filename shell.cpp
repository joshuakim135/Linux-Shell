#include <iostream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>

using namespace std;

vector<string> split(string &str, string delim=" ") {
    size_t start;
    size_t end = 0;
    vector<string> out;
    size_t i = 0;
    
    if ((start = str.find_first_not_of("\"") != string::npos) 
        || ((start = str.find_first_not_of("\'")) != string::npos )) {
        
    }

    while ((start = str.find_first_not_of(delim, end)) != string::npos) {
        if (delim == "|" && i > 0) {
            end = str.find(delim, start);
            out.push_back(str.substr(start + 1, end - start));
        } else {
            end = str.find(delim, start);
            out.push_back(str.substr(start, end - start));
        }
        i++;
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

int main() {
    
    int in_def = dup(0);
    int out_def = dup(1);
    
    vector<int> bgs;

    while (true) {
        dup2(in_def, 0);
        dup2(out_def, 1);
        for (int i = 0; i < bgs.size(); i++) {
           if (waitpid(bgs[i], 0, WNOHANG) == bgs[i]) {
               cout << "Process: " << bgs[i] << " ended" << endl;
               bgs.erase (bgs.begin() + i);
               i--;
           }
        }

        cout << "My Shell$ ";
        string inputline;
        getline(cin, inputline);
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

        // if inputline has "" or ''
        int fd[2];
        int pid = fork();
        
        if (pid == 0) { // if child
            vector<string> parts = split(inputline);
            char** args = vec_to_char_array (parts);
            execvp (args[0], args);
        } else { // if parent
            if (!bg) {
                waitpid(pid, 0, 0); // wait for child process
            } else {
                bgs.push_back(pid);
            }
        }
    }

    return 0; 
}