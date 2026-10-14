#include <iostream>
#include <string>
#include <vector>
#include <cstdio>     // for FILE*, fopen, fclose
#include <unistd.h>   // for getcwd
#include <readline/readline.h>
#include <readline/history.h>


using namespace std;
//for history

const int MAX_HISTORY = 20;      // Maximum commands to store
const int HISTORY_DISPLAY = 10;  // Default number to show

string get_history_file_path() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd)); // Get current working directory
    return string(cwd)+ "/.mysh_history"; // local file
}

const string HISTORY_FILE = get_history_file_path();

void load_history_file() {
    // Create file if it doesn't exist
    FILE* file = fopen(HISTORY_FILE.c_str(), "a");
    if (file)
    {
        fclose(file);
    } 

    //load history from file
    read_history(HISTORY_FILE.c_str());

    // Trim to MAX_HISTORY
    while (history_length > MAX_HISTORY)
     {
        remove_history(0);
    }
}


void add_command_to_history(const char* cmd)
 {
    if (cmd && *cmd)
     {
        add_history(cmd);
        while (history_length > MAX_HISTORY)
         {
            remove_history(0);
        }
 
        if (write_history(HISTORY_FILE.c_str())!= 0)
        {
            cerr<< "Error: could not write history to file" <<endl;
        }
    }
}


void run_history_command(char* argv[], int argc) {
    int n = HISTORY_DISPLAY;
    if (argc >= 2) 
    {
        n = atoi(argv[1]);
        if(n > MAX_HISTORY) n = MAX_HISTORY;
        if(n < 0)
        {
           n = 0;
        }
    }

    int start = max(1, history_length - n + 1);  // history is 1-based
    for (int i = start; i <= history_length; i++)
     {
        HIST_ENTRY* entry = history_get(i);
        if (entry && entry->line)
         {
            cout << entry->line << endl;
        }
    }
}


