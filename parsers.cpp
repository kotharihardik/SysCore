#include <iostream>
#include <unistd.h>
#include <cstring>
#include <errno.h>

#include "header.h"

using namespace std;

extern bool flag_a;
extern bool flag_l;

vector<string> tokenize_for_echo(string input) {
    vector<string> tokens;
    string token = "";
    bool inSingleQuote = false;
    bool inDoubleQuote = false;

    size_t i = 0;
    // Skip leading spaces (left trim)
    while (i < input.size() && (input[i] == ' ' || input[i] == '\t')) i++;

    for (; i < input.size(); i++) {
        char c = input[i];

        // Toggle single quote mode
        if (c == '\'' && !inDoubleQuote) {
            inSingleQuote = !inSingleQuote;
            token += c; // keep the quote
            continue;
        }

        // Toggle double quote mode
        if (c == '"' && !inSingleQuote) {
            inDoubleQuote = !inDoubleQuote;
            token += c; // keep the quote
            continue;
        }

        // Space/tab/newline ends token only if not inside quotes
        if ((c == ' ' || c == '\t' || c == '\n') && !inSingleQuote && !inDoubleQuote)
        {
            if (!token.empty()) {
                tokens.push_back(token);
                token = "";
            }
            continue;
        }

        // Special characters outside quotes
        if ((c == '<' || c == '>' || c == '|' || c == '&') && !inSingleQuote && !inDoubleQuote) 
        {
            if (!token.empty())
             {
                tokens.push_back(token);
                token = "";
            }

            // Check for >> operator
            if (c == '>' && i + 1 < input.size() && input[i + 1] == '>') 
            {
                tokens.push_back(">>");
                i++;
            } 
            else 
            {
                tokens.push_back(string(1, c));
            }
            continue;
        }

        // Normal character (or space inside quotes)
        token += c;
    }

    // Push last token if not empty
    if (!token.empty()) tokens.push_back(token);

    return tokens;
}


vector<string> tokenize(string input) {
    vector<string> tokens;
    string token = "";
    bool inQuotes = false;

    for (int i = 0; i < (int)input.size(); i++) {
        char c = input[i];

        if (c == '"' || c == '\'') {
            inQuotes = !inQuotes;  // toggle quote mode
            continue;              // do not include quotes
        }

        if (!inQuotes && (c == ' ' || c == '\t' || c == '\n')) {
            if (!token.empty()) {
                tokens.push_back(token);
                token = "";
            }
            continue;
        }

        if (!inQuotes && (c == '<' || c == '>' || c == '|' || c == '&')) {
            if (!token.empty()) {
                tokens.push_back(token);
                token = "";
            }

            // handle >> operator
            if (c == '>' && i + 1 < (int)input.size() && input[i + 1] == '>') {
                tokens.push_back(">>");
                i++;
            } else {
                tokens.push_back(string(1, c));
            }
        } else {
            token += c;
        }
    }

    if (!token.empty()) tokens.push_back(token);
    return tokens;
}


vector<string> parser_of_ls(char* argv[], int argc)
{
    vector<string> command;

    for(int i=1; i< argc; i++)
    {
        string argu = argv[i];
        argu.erase(remove(argu.begin(), argu.end(), '\n'), argu.end()); // trim newline

        if(argu[0] == '-')
        {
            for(size_t j=1; j< argu.length(); j++)
            {
                if(argu[j] == 'l')
                {
                    flag_l= true;
                } 
                else if(argu[j] == 'a') {
                    flag_a= true;
                }
                else
                {
                     cerr << "ls: invalid option -- " << argu[j] << "\n";
                }     
            }
        }
        else
        {
            command.push_back(argu);
        }
    }
    //i not provide any argument aftr ls then put .
    if(command.empty()) 
     {
        command.push_back(".");
    }    

    return command;

}