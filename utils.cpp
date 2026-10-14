
#include <unistd.h>
#include <iostream>
#include <limits.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<cstring>
#include<pwd.h>
#include<dirent.h>
#include<algorithm>
#include <fcntl.h>


#include <grp.h>
#include <ctime>

#include "header.h"


using namespace std;

extern string HOME;
extern string PREV_DIR;



string current_directory()
{
     char arr[1000];
     
     if(getcwd(arr , sizeof(arr))) return string(arr);
     return "?";
}

string print_path(const string& cwd) {
    const char* home = getenv("HOME");
    if (home != nullptr) 
    {
        string home_str = home;
        // If cwd starts with $HOME,replace that part with "~"
        if (cwd.rfind(home_str, 0) == 0)
         {  
            return "~" + cwd.substr(home_str.size());
        }
    }
    return cwd;  // otherwise, return full path
}


// Return shell prompt: username@hostname:cwd>
string display_prompt()
{
    string cwd = current_directory();
    string print = print_path(cwd);

    uid_t user_id = geteuid();
    struct passwd* passwd = getpwuid(user_id);

    const char* user;
    if (passwd) 
    {
        user = passwd->pw_name;
    } else {
        user = "user";
    }

    char host_name[256];
    if (gethostname(host_name, sizeof(host_name)) != 0)
     {
        strncpy(host_name, "host", sizeof(host_name));
        host_name[sizeof(host_name)-1] = '\0';
    }

    // Colors (ANSI escape codes)
    const char* GREEN = "\033[32m";
    const char* BLUE  = "\033[34m";
    const char* CYAN  = "\033[36m";
    const char* RESET = "\033[0m";

    // user@host:path$
    string prompt = string(GREEN) + user + "@" + host_name +
                      RESET + ":" +
                    string(BLUE) + print +
                    RESET + string(CYAN) + "$ " + RESET;

    return prompt;
}




void print_long_format(const string &path, string file_name) {
    struct stat st;
    string full_path =path + "/" + file_name;

    if(stat(full_path.c_str(), &st) != 0) {
        cerr << "ls: cannot access '" << file_name << "': " << strerror(errno) << "\n";
        return;
    }

    // File type
    cout <<(S_ISDIR(st.st_mode) ?'d' : '-');

    // Permissions
    cout<< ((st.st_mode & S_IRUSR) ?'r' : '-');
    cout << ((st.st_mode & S_IWUSR) ? 'w' : '-');
    cout << ((st.st_mode & S_IXUSR) ? 'x' : '-');
    cout<<((st.st_mode & S_IRGRP)? 'r' : '-');
    cout << ((st.st_mode & S_IWGRP)? 'w' : '-');
    cout << ((st.st_mode & S_IXGRP)? 'x' : '-');
    cout <<((st.st_mode & S_IROTH) ? 'r' : '-');
    cout<<((st.st_mode & S_IWOTH) ? 'w' : '-');
    cout <<((st.st_mode & S_IXOTH) ? 'x' : '-');

    // Links, owner, group, size
    cout << " "<< st.st_nlink;

    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);
    cout << " " << (pw ? pw->pw_name : "?");
    cout << " " << (gr ? gr->gr_name : "?");
    cout << " " << st.st_size;

    // Modification time
    char timebuf[80];
    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&st.st_mtime));
    cout << " " << timebuf;

    cout << " " << file_name << "\n";
}


void directory_list(string& path, bool flag_a, bool flag_l)
{
    DIR* dir= opendir(path.c_str());

    if(!dir)
    {
         cerr << "ls: cannot access '" << path << "': " << strerror(errno) << "\n";
        return;
    }
    
    vector<string> file_names;
    struct dirent* field;


    while((field = readdir(dir)) != NULL)
    {
        string file_name =field->d_name;

        if(flag_a ==false && file_name[0] == '.')
        {
            continue;
        }
        file_names.push_back(file_name);
    }

    closedir(dir);

    sort(file_names.begin() , file_names.end());

     for(string f : file_names) {
        if(flag_l) {
             print_long_format(path, f);
        } else {
            cout<< f <<"\t";
        }
    }
    cout << endl;
}


//Handles I/O redirection
void handle_redirection(char* argv[], int& argc, int& saved_stdin, int& saved_stdout) {
    saved_stdin= dup(STDIN_FILENO);   //save original stdin
    saved_stdout =dup(STDOUT_FILENO); //save original stdout

    for (int i =0;i <argc;i++) {

        if (strcmp(argv[i],"<")==0)
        {  
            if (i +1 >=argc)
             {
                cerr<< "Error: No input file specified" << endl;
                continue;
            }

            int input_fd =open(argv[i +1],O_RDONLY);
            if (input_fd< 0) 
            {
                perror("Error opening input file");
            } 
            else {
                //reassigning pointer of std in to input_fd
                dup2(input_fd, 0);
                close(input_fd);
            }

            // delete operator and file name from argument array
            for (int j = i;j +2 <=argc; j++) {
                argv[j] =argv[j+ 2];
            }
            argc = argc- 2;
            i= i-1;
        }
        else if (strcmp(argv[i], ">") == 0)
         {
            if (i + 1 >= argc)
             {
                cerr << "Error: No output file specified\n";
                continue;
            }
            int fd = open(argv[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);

            if (fd < 0) 
            {
                perror("Error opening output file");
            } 
            else {
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            for (int j =i; j+2 <= argc; j++) {
                argv[j] = argv[j + 2];
            }
            argc -= 2;
            i--;
        }
        else if (strcmp(argv[i], ">>") == 0) {
            if (i + 1 >= argc)
             {
                cerr << "Error: No output file specified\n";
                continue;
            }
            int fd = open(argv[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd < 0)
             {
                perror("Error opening output file");
            } else {
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            for (int j = i; j + 2 <= argc; j++) {
                argv[j] = argv[j + 2];
            }
            argc -= 2;
            i--;
        }
    }

    argv[argc]= NULL; // null-terminate args
}

// Restore stdin/stdout after command execution
void restore_redirection(int saved_stdin, int saved_stdout) 
{
    dup2(saved_stdin, STDIN_FILENO);
    dup2(saved_stdout, STDOUT_FILENO);

    //close the file
    close(saved_stdin);
    close(saved_stdout);
}


char** to_c_args(const std::vector<std::string>& args) {
    char** c_args = new char*[args.size() +1]; // +1 for NULL terminator
    for (size_t i = 0; i < args.size(); i++)
    {
        c_args[i] = strdup(args[i].c_str()); // copy string to C-style
    }
    c_args[args.size()] = nullptr; // execvp needs NULL terminated list
    return c_args;
}

