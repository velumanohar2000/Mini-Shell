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

void addToHistory(char *command[], int historyIndex);
void history(int history_index, char *historyArray[], char *stringNumber);
int searchCmds(char *cmd, char *cwd, char*tokens[]);
int lookup(char *location, char *cmd);
int execute(char *path, char *tokens[]);

int main()
{

    char s[100];
    char *command_string = (char *)malloc(MAX_COMMAND_SIZE);
    int history_index = 0;
    char **historyArray = malloc(MAX_NUM_HISTORY*sizeof(char));
    for (int i = 0; i < MAX_NUM_HISTORY; i++)
    {
        historyArray[i] = malloc(MAX_COMMAND_SIZE * sizeof(char));
    }

        while (1)
        {
            // Print out the msh prompt
            printf("msh  %s> ", getcwd(s, 100));

            // Read the command from the commandline.  The
            // maximum command that will be read is MAX_COMMAND_SIZE
            // This while command will wait here until the user
            // inputs something since fgets returns NULL when there
            // is no input
            while (!fgets(command_string, MAX_COMMAND_SIZE, stdin));

            /* Parse input */
            char *token[MAX_NUM_ARGUMENTS];

            int token_count = 0;

            // Pointer to point to the token
            // parsed by strsep
            char *argument_ptr;

            char *working_string = strdup(command_string);
            //printf("%s\n", working_string);
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
                {
                    token[token_count] = NULL;
                }
                token_count++;
            }

            // Now print the tokenized input as a debug check
            // \TODO Remove this code and replace with your shell functionality

            // int token_index = 0;
            // for (token_index = 0; token_index < token_count; token_index++)
            // {
            //     printf("token[%d] = %s\n", token_index, token[token_index]);
            // }

            free(head_ptr);

            // TODO: if no input then program will continue
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

            // when user inputs cd
            else if (!(strcmp("cd", token[0])))
            {
                if (chdir(token[1]) != 0)
                {
                    printf("cd: %s: No such file or directory\n", token[1]);
                }
                else 
                {
                    addToHistory(token,history_index);
                }
            }
            else if (('!' == token[0][0] && token[1] == NULL)) //&& isdigit(token[0][2]))
            {
                char stringNumber[10];
                strcpy(stringNumber, token[0] + 1);
                //printf("%s\n", stringNumber);
                history(history_index, historyArray, stringNumber);
                int historyValue;

                

                
            }
            else
            {
                int found = searchCmds(token[0], getcwd(s, 100), token); // TODO: cwd store in variable
                if (!found)
                {
                    printf("%s: Command not found\n", token[0]);
                }
                // TODO history of commands
                // TODO listpids
            }
        }
  return 0;
  // e2520ca2-76f3-90d6-0242ac120003
}
void addToHistory(char *command[], int historyIndex)
{

}

void history(int history_index, char *historyArray[], char *stringNumber)
{
    int length = strlen(stringNumber);
    int historyValue;

    for (int i=0;i<length; i++)
    {
        if (!isdigit(stringNumber[i]))
        {
            printf ("Entered input is not a positive number\n");
            return;
        }
    }

    historyValue = atoi(stringNumber);
    printf("%d\n", historyValue);
    
    if (historyValue > 14)
    {
        printf("Must be between 0 and 14\n");
        return;
    }
    else
    {
        if (historyValue > history_index)
        {
            printf("Command not in history");
        }
        else 
        {

        }
    }

                // if (history_index)
                // {
                //     for (int i = 0; i < history_index; i++)
                //     {

                //     }
                // }
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
            printf("%s %s\n", "FOLDER", direntP->d_name);
            closedir(dir);
            return 1;
        }
      
    }
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