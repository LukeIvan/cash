#include "../include/cash.h"
#include "../include/builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>

int main(void)
{
    /*
        Leave other config options here
    */

    cash_loop();
}

void cash_loop(void)
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
            /*
                Process forking logic here
            */
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

int splitCommandLine(char *cmdBuffer, char *args[])
{
    int nargs = 0; 
    for(int i = 0; i < MAXARGS; i++){
        cmdBuffer = skipChar(cmdBuffer, WHITESPACE); 
        if(*cmdBuffer == '\0' || *cmdBuffer == '\t' || *cmdBuffer == '\n' || *cmdBuffer == '\r')
        {
            return nargs;
        } 
        args[nargs] = cmdBuffer;
        nargs++;
        cmdBuffer = strchr(cmdBuffer, WHITESPACE);
        if(cmdBuffer == NULL) return nargs;
        *cmdBuffer = '\0';
        cmdBuffer++;
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
