#include <iostream>
#include <unistd.h>
#include <cstring>
#include <errno.h>

#include "header.h"

using namespace std;

extern string HOME;
extern string PREV_DIR;

bool check_builtin(const char* command)
{

    if(strcmp(command,"cd")== 0||strcmp(command,"pwd") ==0|| strcmp(command, "echo") ==0)
    {
          return true;
    }
    else return false;
}


int run_cd(char* argv[], int argc)
{
    if(argc > 2)
    {
        cerr << "Invalid argument !!\n";  // ensure newline
        return -1;
    }

    string target;
    if(argc == 1)
    {
        target = HOME.empty() ?current_directory() :HOME;
    }
    else
    {
        string arg1 =argv[1];
        arg1.erase(remove(arg1.begin(), arg1.end(), '\n'), arg1.end()); // trim newline
        if(arg1 == "-")
        {
            if(PREV_DIR.empty()) {
                cerr << "cd: OLDPWD not set\n";
                return -1;
            }

            string curr =current_directory();  // store current dir
            cout << PREV_DIR << endl;           // print the previous dir

            if(chdir(PREV_DIR.c_str()) != 0) {
                cerr << "cd: " << PREV_DIR << ": " << strerror(errno) << "\n";
                return -1;
            }

            swap(PREV_DIR, curr);  // update PREV_DIR to old directory
            return 0;
        }

        // else if(arg1 == "~") target = HOME;

        if(arg1[0] == '~')   // handles cd ~ and cd ~/path
        {
            char* home_env = getenv("HOME");
            string real_home;

            if(home_env != nullptr)
            {
                real_home = string(home_env);
            }
            else real_home=current_directory();

            if(arg1.length() == 1)          // just "~"
                target = real_home;
            else                             // "~/something"
                target = real_home + arg1.substr(1);
        }

        else if(arg1 == ".") target = ".";
        else if(arg1 == "..") target = "..";
        else target = arg1;
    }

    string curr = current_directory();

    if(chdir(target.c_str()) != 0)
    {
        cerr << "cd: " << target << ": " << strerror(errno) << "\n"; // error message
        return -1;
    }

    PREV_DIR = curr;
    return 0;
}



int run_pwd()
{
    string cwd = current_directory();
    cout << cwd << endl;
    return 0;

}

int run_echo(char* argv[] , int argc)
{
    bool new_line = true;
    int start = 1;

    if(argc > 1 && string(argv[1]) == "-n")
    {
        new_line = false;
        start = 2;
    }
    for(int i=start ; i < argc; i++)
    {
        if (i > start) cout << ' ';
        cout << argv[i];
    }

    cout << endl;
    // if(new_line) cout << "\n";
    if(!new_line) cout << " $";
    return 0;

}

void run_builtin_command(char* argv[], int argc)
{
    if(strcmp(argv[0],"cd")== 0)
    {
        run_cd(argv,argc);
    }
    else if(strcmp(argv[0], "pwd") == 0) {
        run_pwd();
    }
    else if(strcmp(argv[0], "echo")== 0) run_echo(argv, argc);
    
}


