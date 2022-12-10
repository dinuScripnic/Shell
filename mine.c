#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define LIMIT 256 // max number of tokens for a command
#define MAXLINE 1024
#define TRUE 1
#define MAX_PROCESSES 100

typedef struct process
{
    int pid;
    char *name;
} process;

void changeDirectory(char **args)
{
    // If we write no path (only 'cd'), then go to the home directory
    if (args[1] == NULL)
    {
        chdir(getenv("HOME"));
        printf("Changed to home directory\n");
    }
    // Else we change the directory to the one specified by the
    // argument, if possible
    else
    {
        if (chdir(args[1]) != 0)
        {
            printf("No such directory\n");
        }
        else
        {
            printf("Changed to directory %s\n", args[1]);
        }
    }
}

void quit(process **processes)
{
    int i = 0;
    // check if there are any running processes
    while (processes[i] != NULL)
    {
        i++;
    }
    if (i == 0)
    {
        exit(0);
    }
    // if there are running processes, then print them
    printf("There are %d running processes:\n", i);
    int j = 0;
    for (j = 0; j < i; j++)
    {
        printf("%d %s\n", processes[j]->pid, processes[j]->name);
    }

    printf("Do you want to kill the running processes? (y/n)\n");
    char answer[2];
    scanf("%s", answer);
    // if the answer is yes, then kill the processes
    if (strcmp(answer, "y") == 0)
    {
        for (j = 0; j < i; j++)
        {
            kill(processes[j]->pid, SIGKILL);
        }
        exit(0);
    }
}

void globalUsage(char **tokens)
{
    int idx = 0;
    // count the number of tokens
    while (tokens[idx] != NULL)
    {
        idx++;
    }
    // if the length of the tokens is 1, then print the text to the screen
    if (idx == 1)
    {
        printf("IMCSH Version 1.1 created by Bertold Vinze, David Bobek and Dinu Scripnic\n");
    }
    // check if token 2 is >
    else if (strcmp(tokens[1], ">") == 0)
    {
        FILE *pFile = NULL;
        // open the file
        pFile = fopen(tokens[2], "a");
        // check if the file was opened
        if (pFile == NULL)
        {
            printf("Something went wrong\n");
        }
        // if the command is globalusage, then write the text to the file
        fprintf(pFile, "IMCSH Version 1.1 created by Bertold Vinze, David Bobek and Dinu Scripnic\n");
        fclose(pFile);
    }
    else
    {
        printf("Something went wrong\n");
    }
}

void executeFunction(char **tokens, process **processes)
{
    int idx = 0;
    FILE *pFile = NULL;
    int background = 0;
    // count the number of tokens
    while (tokens[idx] != NULL)
    {
        idx++;
    }
    if (strcmp(tokens[idx - 1], "&") == 0)
    {
        background = 1;
        idx--;
    }
    char *newTokens[idx];
    int i = 0;
    // copy the tokens to the new array
    for (i = 0; i < idx; i++)
    {
        newTokens[i] = tokens[i];
        if (strcmp(tokens[i], ">") == 0)
        {
            pFile = fopen(tokens[i + 1], "a");
            if (pFile == NULL)
            {
                printf("Something went wrong\n");
            }
            break;
        }
    }
    newTokens[i] = NULL;

    int status;
    // fork a child process
    pid_t pid = fork();
    if (pid == 0)
    {
        if (pFile != NULL)
        {
            dup2(fileno(pFile), 1);
        }
        if (execvp(newTokens[1], newTokens + 1) < 0)
        {
            printf("Error executing command\n");
        }
    }
    else if (pid < 0)
    {
        // error forking
        printf("Error forking\n");
    }
    else
    {
        if (background == 0)
        {
            while (wait(&status) != pid)
                ;
            printf("Child process with id %d terminated\n", pid);
        }
        else
        {
            // add the pid to the background processes array
            int i = 0;
            while (processes[i] != NULL)
            {
                i++;
            }
            processes[i] = (process *)malloc(sizeof(process));
            processes[i]->pid = pid;
            processes[i]->name = newTokens[1];
            printf("Child process with id %d running in background\n", pid);
        }
    }
}

void commandHandler(char **tokens, process **processes)
{
    if (strcmp(tokens[0], "cd") == 0)
    {
        changeDirectory(tokens);
    }
    // check if the command is quit
    else if (strcmp(tokens[0], "quit") == 0)
    {
        quit(processes);
    }
    // check if the command is exec
    else if (strcmp(tokens[0], "exec") == 0)
    {
        executeFunction(tokens, processes);
    }
    // check if the command is globalusage
    else if (strcmp(tokens[0], "globalusage") == 0)
    {
        globalUsage(tokens);
    }
    // if none of the above, then the command is not found
    else
    {
        printf("Command not found\n");
    }
}

int main()
{
    char line[MAXLINE];  // buffer for the user input
    char *tokens[LIMIT]; // array for the different tokens in the command
    int numTokens;

    // create processes array
    process *processes[100];

    for (int i = 0; i < 100; i++)
    {
        processes[i] = NULL;
    }

    // We set our extern char** environ to the environment, so that
    // we can treat it later in other methods

    // We set shell=<pathname>/simple-c-shell as an environment variable for
    // the child
    static char *currentDirectory;
    setenv("shell", getcwd(currentDirectory, 1024), 1); // we set the shell variable to the current directory

    // Main loop, where the user input will be read and the prompt
    // will be printed

    while (TRUE)
    {
        // We print the shell prompt if necessary
        fputs("IMCSH> ", stdout);

        // We empty the line buffer
        memset(line, '\0', MAXLINE);

        // We wait for user input
        fgets(line, MAXLINE, stdin);

        // If nothing is written, the loop is executed again
        if ((tokens[0] = strtok(line, " \n\t")) == NULL)
            continue;

        // We read all the tokens of the input and pass it to our
        // commandHandler as the argument
        numTokens = 1;
        while ((tokens[numTokens] = strtok(NULL, " \n\t")) != NULL)
            numTokens++;
        commandHandler(tokens, processes);
    }

    return 0;
}