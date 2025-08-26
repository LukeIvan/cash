#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include "../include/builtins.h"

/*
    TODO: Convert to hash table
*/

int runBuiltinCommand(char *args[])
{   
    int i = 0;
    while(builtinList[i].builtinFunction != NULL)
    {
        if(strcmp(builtinList[i].name, args[0]) == 0)
        {
            builtinList[i].builtinFunction(args);
            return 1;
        }
        i++;
    }
    return 0;
}

int cdBuiltin(char** args)
{
    if(args[1] == NULL)
    {
        printf("Cash: One or more arguments required for cd\n");
    }
    else if(chdir(args[1]) != 0)
    {
        fprintf(stderr, "Unable to reach directory\n");
    }
    return 1;
}

int lsaFilter(const struct dirent* entryList)
{
    return entryList->d_name[0] != 46;
}

int lsBuiltin(char **args) 
{
    /*
        TODO: STAT FOR DETAILED DIRECTORY INFO && HIGHLIGHTING
    */
    struct dirent **entryList;
    int numEntries;
    char* dir = ".";
    if(args[1] != NULL) // Arguments
    {
        // if(args[1][0] != '-') //Path specified
        // {
        //     numEntries = scandir(args[1], &entryList, NULL, NULL);
        // }
        if(!strcmp(args[1], "-a"))
        {
            test:
            numEntries = scandir(".", &entryList, NULL, NULL);
        }
        else if(!strcmp(args[1], "-la"))
        {
            /*
                TODO: USE STAT FOR DETAILED DIRECTORY INFORMATION
            */
            numEntries = scandir(".", &entryList, NULL, NULL);
        }
        else 
        {
            fprintf(stderr, "Invalid argument for ls!");
            return 0;
        }
    }
    else
    {
        numEntries = scandir(".", &entryList, lsaFilter, NULL);
    }

    if (numEntries <= 0) 
    {
        fprintf(stderr, "No files or directory info available");
        return 0;
    }

    for(int i = 0; i < numEntries; i++)
    {
        printf("%s  ", entryList[i]->d_name);
    }
    printf("\n");

    // Garbage collection
    if (numEntries > 0 && entryList != NULL) {
        for(int i = 0; i < numEntries; i++) {
            if(entryList[i] != NULL) {
                free(entryList[i]);
            }
        }
        free(entryList);
    }
    return 1;
}

int helpBuiltin(char **args) {
    int i = 0;
    if(args[1] != NULL) // Output help for a specific builtin
    {
        while(builtinList[i].builtinFunction != NULL)
        {
            if(!strcmp(builtinList[i].name, args[1]))
            {
                printf("Command: %s\n", args[1]);
                printf("Description: %s\n", builtinList[i].description);
                printf("Usage: %s\n", builtinList[i].usage);
                return 1;
            }
            i++;
        }
        /*
            TODO: Non-Builtin help function logic
        */
        fprintf(stderr, "Cash: Unrecognized command");
        return 0;
    }

    else // Default case
    {
        printf("======= Cash Shell: Builtin Commands =======\n");

        while(builtinList[i].builtinFunction != NULL)
        {
            printf("%d. %s -- %s\n", (i+1), builtinList[i].name, builtinList[i].description);
            printf("Usage: %s\n", builtinList[i].usage);
            i++;
        }
        printf("\nUse 'help <command>' for detailed usage information.\n");
    }

    return 1;
}

int pwdBuiltin(char **args) 
{
    UNUSED(args);
    char *cwd = getcwd(NULL, 0);
    if(cwd == NULL) perror("Couldn't retrieve directory info");
    printf("%s\n", cwd);
    free(cwd);
    return 1;
}

int exportBuiltin(char **args) {
    UNUSED(args);
    printf("export: not implemented yet\n");
    return 1;
}

int aliasBuiltin(char **args) {
    UNUSED(args);
    printf("export: not implemented yet\n");
    return 1;
}

int exitBuiltin(char** args)
{
    UNUSED(args);
    exit(0);
}
