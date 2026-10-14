#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <dirent.h>    // Directory operations
#include <pwd.h>    // User info (getpwuid)
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstdlib>   // General utilities (exit, malloc)
#include <vector>
#include <string>
#include <limits.h>
#include <sys/sysctl.h>
#include <libproc.h>         // macOS process info

#include <readline/readline.h>
#include <readline/history.h>

#include "header.h"



using namespace std;

void run_history_command(char* argv[], int argc);

vector<string> parser_of_ls(char* argv[] , int argc);

bool check_external(const char* command)
{

    if(strcmp(command,"ls")== 0||strcmp(command,"pinfo") ==0||strcmp(command, "search") ==0 || strcmp(command, "history") ==0)
    {
          return true;
    }
    else return false;
}


bool flag_a;
bool flag_l;

void run_ls(char* argv[], int argc)
{
    // Reset flags
    flag_a = false;
    flag_l = false;

    // Get paths and set flags
    vector<string> paths = parser_of_ls(argv, argc);

    for(auto &p :paths)
    {
        // ~ to home
        if(p[0] == '~')
        {
            char* home_env= getenv("HOME");

            string real_home;
            if(home_env != nullptr)
            {
                real_home = string(home_env);
            }
            else real_home= ".";
            if(p.length() == 1) 
            {
                p = real_home;
            }else{
                  p = real_home + p.substr(1);
            }
        }

        struct stat st;
        if(stat(p.c_str(), &st) != 0)
        {
            cerr << "ls: cannot access '" << p << "': " << strerror(errno) << "\n";
            continue;
        }

        if(S_ISDIR(st.st_mode))
        {
            directory_list(p , flag_a, flag_l); 
        }
        else
        {
            if(flag_l) print_long_format(".", p);
            else
            {
                cout << p << "\n";
            }    
        }
    }
}

void run_external_command(char* argv[], int argc)
{
    if(strcmp(argv[0],"ls")== 0)
    {
        run_ls(argv,argc);
    }
    else if(strcmp(argv[0], "pinfo") == 0) {
        run_pinfo();
    }
    else if(strcmp(argv[0], "search")== 0)
    {
        run_file_search(argv);
    } 
     else if(strcmp(argv[0], "history")== 0)
    {
        run_history_command(argv, argc);
    } 
    
}


using namespace std;

void run_pinfo(int pid) {
    if (pid == -1) pid = getpid(); // default: current process

    cout << "pid -- " << pid << endl;

    // Process Status
    char state = '?';
    struct kinfo_proc procInfo;
    size_t size = sizeof(procInfo);

    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, pid};
    if (sysctl(mib, 4, &procInfo, &size, NULL, 0) == 0)
    {
        switch (procInfo.kp_proc.p_stat)
        {
            case SIDL:state = 'I';  break; //idle
            case SRUN: state = 'R'; break;  //running
            case SSLEEP:state = 'S'; break;  // sleeping
            case SSTOP: state = 'T'; break;  // stoping
            case SZOMB: state = 'Z'; break;  // Zombie
        }
    }
    cout<< "Process Status -- " <<state << endl;

    // Memory 
    struct proc_taskinfo taskinfo;
    if (proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &taskinfo, sizeof(taskinfo)) > 0) 
    {
        cout << "memory -- " << taskinfo.pti_resident_size / 1024 << " KB" << endl;
    } 
    else
     {
        cout << "memory -- Unknown" << endl;
    }

    //  Executable Path 
    char pathbuf[PROC_PIDPATHINFO_MAXSIZE];

    if (proc_pidpath(pid, pathbuf, sizeof(pathbuf)) > 0)
     {
        cout << "Executable Path -- " << pathbuf << endl;
    } 
    else {
        cout << "Executable Path -- Unknown" << endl;
    }
}

bool file_search(const string& path , string target)
{

    DIR* dir =opendir(path.c_str());

    if(!dir)
    {
       return false;
    } 

    dirent* read_dir;

    while((read_dir = readdir(dir)) !=NULL)
    {
         string name = read_dir->d_name;

         if(name == "." || name == "..")
         {
             continue;
         }

         if(name==target)
         {
            closedir(dir);
             return true;
         }

         if(read_dir-> d_type == DT_DIR)
         {
            bool val = file_search(path + "/" + name ,target);

            if(val==true){
                closedir(dir);
                return true;
            }
         }
    }

    closedir(dir);
    return false;
}


void run_file_search(char* argv[])
{
      string target = argv[1];
      string path = current_directory();

      bool res =  file_search(path ,target);

      if(res)
      {
         cout << "True" << endl;
      }
      else{
         cout << "False" << endl;
      }
}



// Auto-complete

char* command_generator(const char* text, int state) {
    static vector<string> matches;  // store possible completions
    static size_t list_index;
    static size_t len;

    if (state == 0) {
        matches.clear();
        list_index = 0;
        len = strlen(text);

        // 1) Add builtin commands FIRST
        static const vector<string> builtins = {
            "exit", "history", "search" , "clear"
        };
        for (const auto& cmd : builtins)
        {
            if (strncmp(cmd.c_str(), text, len) == 0) {
                matches.push_back(cmd);
            }
        }

        // 2) Collect matching commands from PATH
        char* path_env = getenv("PATH");
        if (path_env)
        {
            char* path_copy =strdup(path_env);
            char* token= strtok(path_copy, ":");

            while (token != NULL)
             {
                DIR* dir= opendir(token);
                if (dir) 
                {
                    struct dirent* entry;
                    while ((entry = readdir(dir)) != NULL)
                     {
                        if (strncmp(entry->d_name, text, len) == 0) 
                        {
                            matches.push_back(entry->d_name);
                        }
                    }
                    closedir(dir);
                }

                token = strtok(NULL, ":");
            }
             free(path_copy);
        }

        // 3)Collect matching files in current directory
        DIR* dir = opendir(".");
        if (dir)
         {
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) 
            {
                if (strncmp(entry->d_name, text,len) == 0)
                 {
                    matches.push_back(entry->d_name);
                }
            }
            closedir(dir);
        }
}


    // Return matches one by one
    if (list_index < matches.size())
    {
        return strdup(matches[list_index++].c_str()); // readline expects malloced string
    } 
    else {
        return NULL; // No more matches
    }
}


char** my_completion(const char* text, int start, int end) {
    (void)start;
    (void)end;
    rl_attempted_completion_over = 1;  // disable default filename completion

    return rl_completion_matches(text, command_generator);
}





