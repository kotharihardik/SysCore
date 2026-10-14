#ifndef HEADER_H
#define HEADER_H

#include <string>
#include <vector>
using namespace std;

void execute_command(const string& command);

// Externals
bool check_external(const char* command);
void run_external_command(char* argv[], int argc);

// parser 
vector<string> tokenize(string input);
vector<string> tokenize_for_echo(string input);


// individual commands
void run_ls(char** argv, int argc);
void run_pinfo(int pro_id = -1);
void run_file_search(char* argv[]);

// auto-complete
char* command_generator(const char* text, int state);
char** my_completion(const char* text, int start, int end);

// Builtins 
bool check_builtin(const char* cmd);
void run_builtin_command(char** argv, int argc);

// individual commands
int run_cd(char** argv, int argc);
int run_pwd(char** argv);
int run_echo(char** argv, int argc);

//  Utils 
string current_directory();
string display_prompt();
string print_path();

// ls
void print_long_format(const string &path, string file_name);
void directory_list(string& path, bool flag_a, bool flag_l);

void restore_redirection(int saved_stdin, int saved_stdout);
void handle_redirection(char* argv[], int& argc, int& saved_stdin, int& saved_stdout);

char** to_c_args(const std::vector<std::string>& args);


//hstory

void executePipeline(vector<string> &parts);
void load_history_file();
void add_command_to_history(const char* cmd);


#endif // ALL_HEADERS_H
