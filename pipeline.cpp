#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

using namespace std;

void executePipeline(vector<string> &parts) {
    vector<vector<string>> commands; // directly store command args
    string infile, outfile;
    bool append = false;

    vector<string> current;  // temporarily stores parts of the current command
    for (size_t i = 0; i < parts.size(); i++)
     {
        if (parts[i] == "|") 
        {
            if (!current.empty())
             {
                commands.push_back(current);
                current.clear();
            }
        } 
        else if (parts[i] == "<")
         {
            infile = parts[++i];
        } 
        else if (parts[i] == ">") {
            outfile = parts[++i];
            append = false;
        } else if (parts[i] == ">>") 
        {
            outfile = parts[++i];
            append = true;
        } else {
            current.push_back(parts[i]);
        }
    }
    if (!current.empty()) commands.push_back(current);

    int numCmds = commands.size();
    if (numCmds == 0) return;

  // Create pipes for inter-process communication
    vector<int> pipefds(2 * (numCmds - 1));
    for (int i = 0; i < numCmds - 1; i++) 
    {
        // Use pipefds.data() to get pointer to array
        if (pipe(pipefds.data() + i*2) < 0)
        {
            perror("pipe failed");
            return;
        }
    }

    vector<pid_t> pids;
    for (int i = 0; i < numCmds; i++) {
        pid_t pid = fork();
        if (pid == 0) { // child
            if (i == 0 && !infile.empty()) 
            {
                int fd = open(infile.c_str(), O_RDONLY);
                if (fd < 0)
                 { 
                    perror("open infile"); exit(1); 
                 }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

              // handle output redirection for last command
            if (i == numCmds - 1 && !outfile.empty())
             {
                int fd;
                if (append)
                {
                    fd = open(outfile.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
                }    
                else  fd = open(outfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) 
                { 
                    perror("open outfile");
                     exit(1);
                 }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

             // Connect previous command's output to this command's input
            if (i != 0) dup2(pipefds[(i-1)*2], STDIN_FILENO);

             // Connect this command's output to next command's input
            if (i != numCmds - 1) dup2(pipefds[i*2+1], STDOUT_FILENO);

             // Close all pipe file descriptors in child
            for (int j = 0; j < 2*(numCmds-1); j++)
            {
                close(pipefds[j]);
            }    

            vector<char*> argv;
            for (auto &arg : commands[i])
            {
                argv.push_back(const_cast<char*>(arg.c_str()));
            }    
            argv.push_back(nullptr);

            execvp(argv[0], argv.data());
            perror("execvp failed");
            exit(1);
        } 
        else if (pid > 0) {
            pids.push_back(pid);
        } 
        else {
            perror("fork failed");
            return;
        }
    }

    for (int j = 0; j < 2*(numCmds-1); j++) 
    {
        close(pipefds[j]);
    }    

     // Wait for all child processes to finish
    for (pid_t pid : pids)
    {
        waitpid(pid, nullptr, 0);
    } 
}
