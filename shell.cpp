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
    
    int start_not_allowed = -1;
    int end_not_allowed = -1;

    if (((start = str.find("\"")) != string::npos) || ((start = str.find("\'")) != string::npos)) {
        start_not_allowed = start;
    }
    if (start_not_allowed != -1) {
        for (int i = start_not_allowed; i < str.length(); i++) {
            if ((str[i] == '\"') || str[i] == '\'')  {
                end_not_allowed = i;
            }
        }
    }
    
    //if '|' is detected, delim = |
    /*
    if ((start = str.find("|")) != string::npos) {
        if (!(start >= start_not_allowed && start <= end_not_allowed)) {
            delim = "|";
        }
    }*/
    
    while ((start = str.find_first_not_of(delim, end)) != string::npos) {
        if ((start_not_allowed <= start) && (start <= end_not_allowed)) { // if between quotes
            if ((start_not_allowed != -1) && (end_not_allowed != -1)) {
                // add quoted block to vector
                if (end_not_allowed - start_not_allowed > 1) {
                    start_not_allowed += 1;
                    out.push_back(str.substr(start_not_allowed, end_not_allowed - start_not_allowed));
                } else {
                    out.push_back("");
                }

                // set end = end_not_allowed
                end = str.find(delim, end_not_allowed + 1);
                if (end == string::npos) {
                    out.push_back(str.substr(end_not_allowed + 1, str.size()-1));
                }
            } else {
                cout << "Invalid syntax" << endl;
            }
        } else if (delim == "|" && i > 0) {
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
        vector<string> c = split(inputline, "|");

        // fork + exec starts here
        int fd[2];
        int pid = fork();
        if (pid == -1) {
            perror("fork");
        } else if (pid == 0) { // if child
            vector<string> parts = split(inputline);
            for (string word : parts) {
                cout << word << endl;
            }


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

    return 0; 
}