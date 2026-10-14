#include <unistd.h>       // POSIX API: fork(), exec(), getcwd(), chdir(), pipe(), etc.
#include <sys/types.h>    // Defines types like pid_t, uid_t, etc.
#include <limits.h>       // Defines limits for integral types, PATH_MAX for max path length
#include <cstring>        // C-style string functions: strcpy, strlen, strcmp, etc.
#include <string>         // C++ string class
#include <pwd.h>          // For passwd struct, getpwuid() to get user info
#include <sys/wait.h>     // wait(), waitpid(), macros like WIFEXITED, WIFSIGNALED
#include <vector>         // C++ dynamic array std::vector
#include <iostream>       // scin,cout, cerr
#include <signal.h>       // Signal handling: SIGINT, SIGTSTP, signal()
#include <csignal>        // C++ version of signal handling

#include <readline/readline.h>  // GNU Readline: input line reading with editing
#include <readline/history.h>   // GNU Readline: maintain command history

#include "header.h"       // Custom header file: function declarations & macros





string HOME;
string PREV_DIR;

struct Job {
    int id;
    pid_t pid;
    string command;
};


using namespace std;
vector<Job> jobs;
int job_counter = 1;


// Keep track of foreground job PID (0 if none running)
pid_t fg_pid = 0;

//CTRL-C Handler
void sigint_handler(int sig) {
    (void) sig;
    if (fg_pid > 0) {
        // Forward SIGINT to foreground child
        kill(fg_pid, SIGINT);
    } else {
        // No foreground job → refresh prompt immediately
        write(STDOUT_FILENO, "\n", 1);   // move to new line
        rl_on_new_line();                // tell readline we’re on a new line
        rl_replace_line("", 0);          // clear buffer
        rl_redisplay();                  // show prompt instantly
    }
}


// CTRL+Z (SIGTSTP handler)
void sigtstp_handler(int sig) {
    (void)sig;

    if (fg_pid > 0) {
        // Send stop signal to foreground job
        kill(fg_pid, SIGTSTP);

        // Save job with actual command (optional: track from global state)
        string cmd = "foreground-job";  
        jobs.push_back({job_counter, fg_pid, cmd});

        cout << "\n[" << job_counter << "] Stopped\t" << cmd << endl;
        job_counter++;

        fg_pid = 0;  // reset foreground PID
    }

    // Always refresh prompt instantly
    write(STDOUT_FILENO, "\n", 1);   // move cursor to new line
    rl_replace_line("", 0);
    rl_on_new_line();                // tell readline we’re at a fresh prompt
    rl_redisplay();                  // redraw prompt
}



void sigchld_handler(int sig) {
    (void)sig;
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // remove finished background jobs from list
        for (auto it =jobs.begin(); it != jobs.end(); ++it)
         {
            if (it->pid ==pid) 
            {
                cout << "\n[" << it->id << "] + done " << it->command << "\n";
                jobs.erase(it);
                break;
            }
        }
    }
    rl_on_new_line();
    rl_redisplay();
}





// check_background_jobs no longer needs to print
void check_background_jobs() {
    int status;
    for (auto it = jobs.begin(); it != jobs.end();) {
        pid_t result = waitpid(it->pid, &status, WNOHANG);
        if (result > 0) {
            // Already handled in signal, just erase safely
            it =jobs.erase(it);
        } else
         {
            ++it;
        }
    }
}




void execute_command(const string& command)
 {
    //  tokenize the command into words
    vector<string> parts = tokenize(command);  
    if (parts.empty()) 
    {
        return;
    }

    // detect background execution (& at end)
    bool background_exec = false;
    if (parts.back() == "&") {
        background_exec = true;
        parts.pop_back();
    }

    //  convert to argv style for execvp / builtins
    char** argv =to_c_args(parts);   // convert vector string format c style argv 
    int argc = parts.size();

    if (strcmp(argv[0], "echo") == 0) 
   {
        vector<string> parts = tokenize_for_echo(command); 
        argv =to_c_args(parts);   // convert vector string format c style argv 
        argc = parts.size(); 
    }

    // setup signal for child cleanup
    signal(SIGCHLD, sigchld_handler);

    //  save stdin/stdout and handle redirection
    int saved_stdin, saved_stdout;
    handle_redirection(argv, argc, saved_stdin, saved_stdout);

   if (strcmp(argv[0], "exit") == 0) 
   {
    exit(0);
    }

    // check builtin
    if (check_builtin(argv[0]))
    {
        run_builtin_command(argv, argc);
        restore_redirection(saved_stdin, saved_stdout);
        return;
    }

    //  check external
    if (check_external(argv[0])) {
        run_external_command(argv, argc);
        restore_redirection(saved_stdin, saved_stdout);
        return;
    }

    // fallback  run as system command
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
    } 
    else if (pid == 0) {
        // child
        execvp(argv[0], argv);
        cerr << argv[0] << ": command not found" << endl;
        exit(127);
    } 
    else {
        // parent
        if (background_exec) {
            string cmd_str;
            for (size_t i = 0; i < parts.size(); i++) {
                if (i > 0) cmd_str += " ";
                cmd_str += parts[i];
            }
            jobs.push_back({job_counter, pid, cmd_str});
            cout << "[" << job_counter << "] " << pid << "\n";
            job_counter++;
        } else {
             // Foreground: set fg_pid
            fg_pid = pid;
            int status;
            waitpid(pid, &status, 0);
            fg_pid = 0; // reset after child finishes

             cout << flush;
             cout << "\n";
            
        }
    }

    //  restore stdin/stdout
    restore_redirection(saved_stdin, saved_stdout);
}



void setup_readline() {
    rl_completion_append_character = '\0';
    rl_completion_query_items = 100;

    // Default behaviour: first TAB -> prefix, double TAB -> list
    rl_bind_key('\t', rl_complete);

    // Like bash: cycle via ESC+TAB
    // rl_bind_keyseq("\\e\t", rl_menu_complete);
}




int main() {
    string HOME = current_directory();
    string PREV_DIR = HOME;

    // Attach handlers
    signal(SIGINT, sigint_handler);   // Replace with your sigint_handler -- 2
    signal(SIGTSTP, sigtstp_handler);  // Replace with yoursigtstp_handler -- 20
    signal(SIGCHLD, sigchld_handler);  // Replace with your sigchld_handler --17

    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Hook autocomplete
    rl_attempted_completion_function = my_completion; 
    setup_readline();
    load_history_file();

    // Ensure stdout is line-buffered (fixes multiple prompt issue)
    setvbuf(stdout, NULL, _IOLBF, 0);

    while (true) {
        string prompt = display_prompt();
        cout << flush;
        char* input = readline(prompt.c_str());

        if (!input) {
            exit(0);
        }

        if (*input)
        {
             add_command_to_history(input);
        } 

        char line[1024];
        strncpy(line, input, sizeof(line));
        line[sizeof(line) - 1] = '\0';

        // Split by ';' for multiple commands
        char* token = strtok(line, ";");
        while (token != NULL) {
            string cmd_line = token;
            vector<string> parts = tokenize(cmd_line);

            bool hasPipe = false;
            for (auto &p : parts)
            {
                if (p== "|")
                { 
                    hasPipe = true; 
                    break;
                }
            } 

            if (hasPipe)
             {
                executePipeline(parts);
            } 
            else {
                execute_command(cmd_line);
            }

            token = strtok(NULL, ";");
        }

        check_background_jobs();
        free(input);
    }

    return 0;
}