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

string awkTrim(string input) {
    if (input.find("\'") == -1) {
        return input;
    }
    size_t second = input.find_last_of("\'");
    input = input.substr(1, second - 1);
    while (input.find(" ") != -1) {
        size_t space = input.find_last_of(" ");
        input = input.substr(0, space) + input.substr(space + 1);
    }

    return input;
}

string trim (string input) {
    string trimmed;
    size_t first = input.find_first_not_of(" ");
    size_t last = input.find_last_not_of(" ");
    input = input.substr(first);
    trimmed = input.substr(0, last + 1);
    // so far, the whitespace at the beginning and end has been removed
    // now we must remove quotation marks "if found"
    if (trimmed.find("\"") != string::npos)
    {
        // if we find quotation marks
        string trimmed2;
        size_t first = trimmed.find_first_of("\""); // find index of first quotation mark
        if (first > 0)
        {
            trimmed2 = trimmed.substr(0, first);
        }
        trimmed = trimmed.substr(first + 1);
        size_t second = trimmed.find_last_of("\""); //find index of second quotation mark
        trimmed2.append(trimmed.substr(0, second));

        return trimmed2; // returns trimmed string with no quotation marks
    }
    return trimmed; // returns trimmed string
    // trim beginning
    /*
    while (input.size() > 0) {
        if (input[0] == ' ') {
            input.erase(0,1);
        } else {
            break;
        }
    }

    // trim ending
    while (input.size() > 0) {
        if (input[input.size()-1] == ' ') {
            input.erase(input.size()-1, 1);
        } else {
            break;
        }
    }

    return input;
    */
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
    /*
    vector<string> result;
    string word = "";
    int num = 0;
    str = str + delim;

    if (isPipeBetweenQuotes(str)) {
        delim = ' ';
    }

    if (delim == ' ') {
        for (int i = 0; i < str.size(); i++) {
            char current = str[i];
            if (current == '\"') {
                size_t endDQ = str.find('\"', i + 1);
                result.push_back(str.substr(i+1, endDQ - i - 1));
                i = endDQ;
            } else if (current == '\'') {
                size_t endQuote = str.find("\'", i + 1);
                result.push_back(str.substr(i+1, endQuote - i - 1));
                i = endQuote;
            } else if (current == '{') {
                size_t endBrace = str.find('}', i + 1);
                result.push_back(str.substr(i+1, endBrace - i -1));
                i = endBrace;
            } else if (current == delim) {
                if (word.size() != 0) {
                    result.push_back(word);
                }
                word = "";
            } else {
                word += current;
            }
        }
    }

    if (delim == '|') {
        for (int i = 0; i < str.size(); i++) {
            if (str[i] != delim) {
                word = word + str[i];
            } else {
                if (word.size() != 0) {
                    result.push_back(word);
                }
                word = "";
            }
        }
    }

    for (string s : result) {
        trim(s);
    }
    return result;
    */
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
    int in_def = dup(0);
    int out_def = dup(1);
    
    vector<int> zombie;
    string previousCmd;
    vector<string> directories;

    while (true) {
        // reset file descriptor
        dup2(in_def, 0);
        dup2(out_def, 1);

        // clean up background process
        for (int i = 0; i < zombie.size(); i++) {
           if (waitpid(zombie[i], 0, WNOHANG) == zombie[i]) {
               cout << "Process: " << zombie[i] << " ended" << endl;
               zombie.erase (zombie.begin() + i);
               i--;
           }
        }

        // shell prompt
        string userName = getlogin();
        cout << userName << "$ ";
        char buffer[1000];
        string currentDir = getcwd(buffer, sizeof(buffer)); // gets the current working directory and stores it in variable
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

        // do this for each pipe
        for (int i = 0; i < c.size(); i++) {
            int fds[2];
            pipe(fds);

            // & background processes
            bool bg = false;
            int bgp = c[i].find("&");
            if (bgp != string::npos) {
                c[i] = c[i].substr(0, bgp - 1);
                bg = true;
                cerr << "Found a background process" << endl;
            }

            int pid = fork();
            if (pid == 0)
            {
                c[i] = trim(c[i]);
                // awk function testing
                if (c[i].find("awk") == 0)
                {
                    c[i] = "awk " + awkTrim(c[i].substr(c[i].find("\'")));
                }

                // ls > a.txt and < a.txt
                int pos1 = inputline.find('>'); // gets the index where ">" is found
                int pos2 = inputline.find('<'); // gets the index where "<" is found
                if (pos1 >= 0)
                {
                    // if the ">" is found
                    string command = inputline.substr(0, pos1);
                    string filename = trim(inputline.substr(pos1 + 1)); // substrings all the way to the end from index pos+1

                    c[i] = command;
                    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_RDONLY, S_IWUSR | S_IRUSR);
                    dup2(fd, 1);
                    close(fd);
                }

                if (pos2 >= 0)
                {
                    // if the "<" is found
                    string command = inputline.substr(0, pos2);
                    string filename = trim(inputline.substr(pos2 + 1)); // substrings all the way to the end from index pos+1

                    c[i] = command;
                    int fd = open(filename.c_str(), O_RDONLY | O_CREAT, S_IWUSR | S_IRUSR);
                    dup2(fd, 0);
                    close(fd);
                }

                if (i < c.size() - 1) {
                    dup2(fds[1], 1);
                }
                
                vector<string> parts = split(c[i]);
                char **args = vec_to_char_array(parts);
                execvp(args[0], args);
            }

            else {
                if (!bg) {
                    if (i == c.size() - 1) {
                        waitpid(pid, 0, 0);
                    } else {
                        zombie.push_back(pid);
                    }
                } else {
                    zombie.push_back(pid);
                }

                dup2(fds[0], 0);
                close(fds[1]);
            }
        }
    }
    return 0; 
}