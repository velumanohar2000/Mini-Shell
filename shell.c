// The MIT License (MIT)
// 
// Copyright (c) 2016 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <ctype.h>


#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports four arguments
#define MAX_NUM_HISTORY 14

void addToHistory(const char *command,
                  char historyArray[MAX_NUM_HISTORY][MAX_COMMAND_SIZE]);
char* getCommandHistory(char historyArray[MAX_NUM_HISTORY][MAX_COMMAND_SIZE], int history_cmd);

int checkValidHistoryCommand(int history_index, char *stringNumber);
void getCommand();
int searchCmds(char *cmd, char *cwd, char*tokens[]);
int lookup(char *location, char *cmd);
int execute(char *path, char *tokens[]);

int hist_start = 0;
int hist_count = 0;


int main()
{

    char s[100];
    char *command_string = (char *)malloc(MAX_COMMAND_SIZE);
    char historyArray[MAX_NUM_HISTORY][MAX_COMMAND_SIZE];
    int historyCmdValue = 0;

    int getCommandFromHistory = 0;
    char *originalCommand = (char *)malloc(MAX_COMMAND_SIZE);


    while (1)
    {
        // Pointer to point to the token
        // parsed by strsep
        char *working_string;
        if (getCommandFromHistory)
        {
            working_string = strdup(historyArray[historyCmdValue]);
            getCommandFromHistory = 0;
        }
        else
        {
            // Print out the msh prompt
            printf("msh  %s> ", getcwd(s, 100));
            while (!fgets(command_string, MAX_COMMAND_SIZE, stdin));
            /* Parse input */
            working_string = strdup(command_string);
        } 
        strcpy(originalCommand, working_string);
        char *token[MAX_NUM_ARGUMENTS];
        int token_count = 0;

        char *argument_ptr;

        // we are going to move the working_string pointer so
        // keep track of its original value so we can deallocate
        // the correct amount at the end
        char *head_ptr = working_string;
        
        // Tokenize the input strings with whitespace used as the delimiter
        while (((argument_ptr = strsep(&working_string, WHITESPACE)) != NULL) &&
                (token_count < MAX_NUM_ARGUMENTS))
        {
            token[token_count] = strndup(argument_ptr, MAX_COMMAND_SIZE);
            if (strlen(token[token_count]) == 0)
                token[token_count] = NULL;
            token_count++;
        }
        
        free(head_ptr);

        if (token[0] == NULL)
        {
            continue;
        }

        // when user inputs quit
        else if ((!(strcmp("quit", token[0]))) || !(strcmp("exit", token[0])))
        {
            printf("Goodbye\n");
            return 0;
        }

        else if (!(strcmp("history", token[0])))
        {
            // add to circular buffer
            addToHistory(originalCommand, historyArray);
            // print from oldest (hist_start) up to hist_count entries
            for (int i = 0; i < hist_count; i++) {
                int idx = (hist_start + i) % MAX_NUM_HISTORY;
                printf("%2d: %s", i, historyArray[idx]);
            }
        }
        else if (!(strcmp("cd", token[0])))
        {
            if (chdir(token[1]) != 0)
                printf("cd: %s: No such file or directory\n", token[1]);
            else 
                addToHistory(originalCommand, historyArray);
        }
        else if (('!' == token[0][0] && token[1] == NULL))
        {
            char stringNumber[10];
            strcpy(stringNumber, token[0] + 1);
            historyCmdValue = checkValidHistoryCommand(hist_count, stringNumber);
            getCommandFromHistory = (historyCmdValue >= 0);
        }
        else
        {
            int found = searchCmds(token[0], getcwd(s, 100), token);
            if (!found)
                printf("%s: Command not found\n", token[0]);            
            else 
                addToHistory(originalCommand, historyArray);
        }
        
    }

    free(command_string);
    free(originalCommand);
    return 0;
}
// push a new entry into the circular history buffer
void addToHistory(const char *command,
                  char historyArray[MAX_NUM_HISTORY][MAX_COMMAND_SIZE])
{
    int pos = (hist_start + hist_count) % MAX_NUM_HISTORY;
    // copy the command (truncate if necessary)
    strncpy(historyArray[pos], command, MAX_COMMAND_SIZE-1);
    historyArray[pos][MAX_COMMAND_SIZE-1] = '\0';

    if (hist_count < MAX_NUM_HISTORY) {
        // buffer not yet full, just grow it
        hist_count++;
    } else {
        // buffer full, advance start (overwrite oldest)
        hist_start = (hist_start + 1) % MAX_NUM_HISTORY;
    }
}

char* getCommandHistory(char historyArray[MAX_NUM_HISTORY][MAX_COMMAND_SIZE], int history_cmd)
{
    printf("%s", historyArray[history_cmd]);
    return historyArray[history_cmd];
}

int checkValidHistoryCommand(int history_index, char *stringNumber)
{
    int length = strlen(stringNumber);
    int historyValue;

    for (int i=0;i<length;i++)
    {
        if (!isdigit(stringNumber[i]))
        {
            printf ("Entered input is not a positive number\n");
            return -1;
        }
    }

    historyValue = atoi(stringNumber);
    
    if (historyValue > MAX_NUM_HISTORY)
    {
        printf("Must be between 0 and %d\n", MAX_NUM_HISTORY - 1);
        return -1;
    }
    else
    {
        if (historyValue >= history_index)
        {
            printf("Command not in history\n");
            return -1;
        }
        else 
            return historyValue;
    }

                
}

int searchCmds(char *cmd, char *cwd, char *tokens[])
{
    char *location[] = {cwd, "/usr/local/bin", "/usr/bin", "/bin"};
    char path[100];

    for (int i = 0; i < 4; i++)
    {
        if (lookup(location[i], cmd))
        {
            strcpy(path, location[i]);
            strcat(path, "/");
            strcpy(path, strcat(path, cmd));
            execute(path,tokens);
            return 1;
        }
    }
    return 0;
}

int lookup(char *location, char *cmd)
{
    struct dirent *direntP;
    DIR *dir;
    dir = opendir(location);

    if (dir == NULL)
    {
        printf("Cannot Open Directory: %s", location);
        return 0;
    }

    while((direntP=readdir(dir))!= NULL)
    {
        if(strcmp(direntP->d_name, cmd)==0 ) 
        {
            closedir(dir);
            return 1;
        }
    }
    closedir(dir);
    return 0;
}
        
int execute(char* path, char *tokens[])
{
    pid_t child_pid = fork();
    int status;
    if (child_pid == -1)
    {
        printf("Failed to fork\n");
        exit(0);
    }
    else if (child_pid == 0)
    {
        execvp(path,tokens);
        return 1;
    }
    waitpid(child_pid, &status, 0);
}