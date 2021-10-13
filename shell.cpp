#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <vector>
#include <sstream>

using namespace std;

bool isPipeBetweenQuotes (string input) {
    size_t startDQ = input.find_first_of('\"');
    size_t startQ = input.find_first_of('\'');
    size_t endDQ = input.find_last_of('\"');
    size_t endQ = input.find_last_of('\'');
    size_t posPipe = input.find_first_of('|');
    if (startDQ < posPipe && posPipe < endDQ) {
        return true;
    } else if (endQ < posPipe && posPipe < endQ) {
        return true;
    }
    return false;
}

string trimAwk(string input) {
    if (input.find("\'") == string::npos) {
        return input;
    }
    int last = input.find_last_of("\'");
    input = input.substr(1, last - 1);
    while (input.find(" ") != string::npos) {
        int space = input.find_last_of(" ");
        input = input.substr(0, space) + input.substr(space + 1);
    }
    return input;
}

string trimQuoteHelper (size_t first, size_t last, string trimmedInput) {
    // trim any "quotes"
    if (trimmedInput.find("\"") != string::npos) {
        // if we find quotation marks
        string quoteTrimmed;
        size_t first = trimmedInput.find_first_of("\"");
        if (first > 0) {
            quoteTrimmed = trimmedInput.substr(0, first);
        }
        trimmedInput = trimmedInput.substr(first + 1);
        size_t second = trimmedInput.find_last_of("\"");
        quoteTrimmed.append(trimmedInput.substr(0, second));

        return quoteTrimmed;
    }
    return trimmedInput;
}

string trim (string input) {
    // trim whitespace at beginning and end
    string trimmedInput;
    size_t first = input.find_first_not_of(" ");
    size_t last = input.find_last_not_of(" ");
    input = input.substr(first);
    trimmedInput = input.substr(0, last + 1);

    // returns trimmedInput string
    return trimQuoteHelper (first, last, trimmedInput);
}

vector<string> split(string str, char delim = ' ') {
    vector<string> out;
    size_t start;
    size_t end = 0;
    while ((start = str.find_first_not_of(delim, end)) != string::npos)
    {
        end = str.find(delim, start);
        out.push_back(str.substr(start, end - start));
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

void fileOutputRedirect (string inputline, int fileOutput, vector<string> &c, int i) {
    string command = inputline.substr(0, fileOutput);
    string filename = trim(inputline.substr(fileOutput + 1)); // substrings all the way to the end from index pos+1

    c[i] = command;
    int fd = open(filename.c_str(), O_RDONLY | O_CREAT, S_IWUSR | S_IRUSR);
    dup2(fd, 0);
    close(fd);
}

void fileInputRedirect (string inputline, int fileInput, vector<string> &c, int i) {
    string command = inputline.substr(0, fileInput);
    string filename = trim(inputline.substr(fileInput + 1)); // substrings all the way to the end from index pos+1

    c[i] = command;
    int fd = open(filename.c_str(), O_CREAT | O_WRONLY | O_TRUNC | O_RDONLY, S_IWUSR | S_IRUSR | S_IROTH);
    dup2(fd, 1);
    close(fd);
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
        if (inputline == string("exit")) {
            cout << "Bye!1 End of shell" << endl;
            break;
        }

        // check if it's a 'cd' command
        if (inputline.length() > 2) {
            if (inputline.find("cd") == 0) {
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

        // check if it's a echo command
        if (inputline.length() > 6) {
            if (inputline.find("echo") == 0) {
                if (inputline[5] == '\"' || inputline[5] == '\'') {
                    bool error = false;
                    size_t end = inputline.find_last_of("\"");
                    if (end == string::npos) {
                        end = inputline.find_last_of("\'");
                        if (end == string::npos) {
                            error = true;
                        }
                    }
                    if (!error) {
                        cout << inputline.substr(6, end - 6) << endl;
                        continue;
                    }       
                }
            }
        }
        
        inputline = trim(inputline);
        vector<string> c = split(inputline, '|');

        // for each pipe...
        for (int i = 0; i < c.size(); i++) {
            int fds[2];
            pipe(fds);

            // check for '&'
            bool bg = false;
            int bgp = c[i].find("&");
            if (bgp != string::npos) {
                c[i] = c[i].substr(0, bgp);
                bg = true;
                cerr << "Found a background process" << endl;
            }
            
            // fork + exec starts here
            int pid = fork();
            if (pid == 0) {
                c[i] = trim(c[i]);

                //  check for awk
                if (c[i].find("awk") == 0) {
                    c[i] = "awk " + trimAwk(c[i].substr(c[i].find("\'")));
                }

                // file input output redirection
                int fileInput = inputline.find('>');
                int fileOutput = inputline.find('<');
                if ((fileInput != string::npos) || (fileOutput != string::npos)) {
                    if ((fileInput >= 0 && fileOutput >= 0) && (fileInput > fileOutput)) {
                        fileOutputRedirect(inputline, fileOutput, c, i);
                        fileInputRedirect(inputline, fileInput, c, i);
                    } else if ((fileInput >= 0 && fileOutput >= 0) && (fileOutput > fileInput)) {
                        fileInputRedirect(inputline, fileInput, c, i);
                        fileOutputRedirect(inputline, fileOutput, c, i);
                    } else {
                        if (fileOutput >= 0) {
                            fileOutputRedirect(inputline, fileOutput, c, i);
                        }
                        if (fileInput >= 0) {
                            fileInputRedirect(inputline, fileInput, c, i);
                        }
                    }
                }

                // redirect stdout to fd[1] so that it can write to the other side
                if (i < c.size() - 1) {
                    dup2(fds[1], 1);
                }
                
                vector<string> parts = split(c[i]);
                char **args = vec_to_char_array(parts);
                execvp(args[0], args);
            }

            else {
                if (!bg) {
                     waitpid(pid, 0, 0);
                } else {
                    bgs.push_back(pid);
                }

                dup2(fds[0], 0);
                close(fds[1]);
            }
        }
    }
    return 0; 
}