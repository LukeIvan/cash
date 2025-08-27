#include "../include/cash.h"
#include "../include/builtins.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <sys/wait.h>
#include <unistd.h>
#include <termios.h>

struct termios orig_termios;

int main(void)
{
    /*
        Leave other config options here
    */

    /*
        TODO: Handle SIGINT
    */
    cashLoop();
}

void cashLoop(void)
{
    char *cwd = getcwd(NULL, 0);
    if(cwd == NULL) perror("Couldn't retrieve directory info");
    char* reducedCwd = reduceTilde(cwd);

    char cmdBuffer[CMD_BUFFSIZE];
    char *args[MAXARGS+1];
    char *allocArgs[MAXARGS+1] = {0};

    printf("%s>", reducedCwd);
    fflush(stdout);

    // Readline essentially
    while(fgets(cmdBuffer,CMD_BUFFSIZE,stdin) != NULL)
    {
        int cmdLen = strlen(cmdBuffer);
        if (cmdBuffer[cmdLen-1] == '\n' || cmdBuffer[cmdLen-1] == EOF){
            cmdBuffer[cmdLen-1] = '\0';
	    }
        int nargs = splitCommandLine(cmdBuffer, args);
        args[nargs] = NULL;

        for(int i = 1; i < nargs; i++) // Expand '~' in any arguments to home directory
        {
            char* expanded = expandTilde(args[i]);
            if(expanded != args[i]) {
                allocArgs[i] = expanded;
                args[i] = expanded;
            }
        }

        // TODO: Add config option here for debugging flags. Not in TOML but maybe w Macro?
        // for(int i = 0; i < nargs; i++)
        // {
        //     printf("Argument %d: %s\n", i, args[i]);
        // }

        int builtinSuccess = runBuiltinCommand(args);
        if(!builtinSuccess)
        {
           if(!runCommand(args))
           {
                printf("cash: \"%s\" not recognized. Try \"help\" for options.\n", args[0]);
           }
        }

        // Garbage collection section
        for(int i = 0; i < MAXARGS+1; i++) 
        {
            if(allocArgs[i] != NULL) {
                free(allocArgs[i]);
                allocArgs[i] = NULL;
            }
        }

        if(reducedCwd != cwd) {
            free(reducedCwd);
        }
        reducedCwd = NULL;
        cwd = NULL;
        // End garbage collection section 

        cwd = getcwd(NULL, 0);
        if(cwd == NULL) perror("Couldn't retrieve directory info");
        reducedCwd = reduceTilde(cwd);

        printf("%s>", reducedCwd);
        fflush(stdout);
    }

    if(reducedCwd != cwd) {
        free(reducedCwd);
    }
    free(cwd);
}

int runCommand(char *args[])
{
    pid_t pid;
    int childStatus;

    pid = fork();
    if(pid == 0)
    {
        /*
            INTERNAL OF CHILD PROCESS;
        */
        if(execvp(args[0], args) == -1) // Exec process
        {
            return 0;
        }
        // Once process is over, return to shell
        exit(0);
    }
    else if(pid < 0)
    {
        perror("Cash: fork");
        return 0;
    }
    else
    {
        do {
        {
            waitpid(pid, &childStatus, 0);
        }
        }while (!WIFEXITED(childStatus) && !WIFSIGNALED(childStatus));
    }
    return 1;
}

int splitCommandLine(char *cmdBuffer, char *args[])
{
    int nargs = 0; 
    char *start, *end;
    for(int i = 0; i < MAXARGS; i++)
    {
        cmdBuffer = skipChar(cmdBuffer, WHITESPACE); 
        if(*cmdBuffer == '\0' || *cmdBuffer == '\t' || *cmdBuffer == '\n' || *cmdBuffer == '\r')
        {
            return nargs;
        }

        if(*cmdBuffer == '"') 
        {
            cmdBuffer++;
            start = cmdBuffer;
            
            end = strchr(cmdBuffer, '"');
            if(end == NULL) 
            {
                fprintf(stderr, "Cash: Unclosed double quote\n");
                return 0;
            }
            
            *end = '\0';
            args[nargs] = start;
            nargs++;
            cmdBuffer = end + 1;
        }
        else 
        {
            args[nargs] = cmdBuffer;
            nargs++;

            while(*cmdBuffer != '\0' && *cmdBuffer != WHITESPACE && *cmdBuffer != '"') cmdBuffer++;

            if(cmdBuffer == NULL) return nargs;
            if(*cmdBuffer == '"') continue;
            
            *cmdBuffer = '\0';
            cmdBuffer++;
        }
    }
    fprintf(stderr, "Cash: Maximum arguments (%d) exceeded\n", MAXARGS);
    return 0;
}

inline char *skipChar(char *charPtr, char skip)
{
    if (skip == '\0') return charPtr;
    while(*charPtr == skip){
        charPtr++;
    }
    return charPtr;
}

char* expandTilde(char* path) {
    if(path[0] != '~') return path;
    
    if(path[1] == '\0' || path[1] == '/') {
        struct passwd *pw = getpwuid(getuid());
        if(pw == NULL || pw->pw_dir == NULL)
        {
            fprintf(stderr, "Unable to locate home directory\n");
            return path;
        } 
        
        if(path[1] == '\0') // ~
        {
            return strdup(pw->pw_dir);
        } 
        else // ~/path/ext
        {
            size_t home_len = strlen(pw->pw_dir);
            size_t path_len = strlen(path) - 1;
            char* expanded = malloc(home_len + path_len + 1);
            strcpy(expanded, pw->pw_dir);
            strcat(expanded, path + 1);
            return expanded;
        }
    }
    fprintf(stderr, "Username may not be supported or path may not exist\n");
    return path;  // ~username not supported or expansion failed
}

char* reduceTilde(char* path) {
    if(path[0] != '/' || path == NULL) return path;

    struct passwd *pw = getpwuid(getuid());
    if(pw == NULL || pw->pw_dir == NULL)
    {
        fprintf(stderr, "Unable to locate home directory\n");
        return path;
    }

    size_t homeLength = strlen(pw->pw_dir);
    size_t pathLength = strlen(path);

    if(pathLength >= homeLength && !strncmp(path, pw->pw_dir, homeLength)) 
    {
        if(pathLength == homeLength || path[homeLength] == '/') {
            char* reduced = malloc(pathLength - homeLength + 2);
            if(reduced == NULL) {
                fprintf(stderr, "Failed to get memory for replaced string\n");
                return path;
            }
            
            reduced[0] = '~';
            strcpy(reduced + 1, path + homeLength);
            return reduced;
        }
    }
    
    return path;
}

// void enableISIG(void)
// {
//     tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
// }

// void disableISIG(void)
// {
//     tcgetattr(STDIN_FILENO, &orig_termios);
//     atexit(enableISIG);

//     struct termios new = orig_termios;
//     new.c_lflag &= ~(ISIG);

//     tcsetattr(STDIN_FILENO, TCSAFLUSH, &new);
// }

