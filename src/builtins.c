#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <limits.h>
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

void printFilePermissions(mode_t mode) 
{
    if (S_ISDIR(mode)) printf("d");
    else if (S_ISLNK(mode)) printf("l");
    else if (S_ISFIFO(mode)) printf("p");
    else if (S_ISSOCK(mode)) printf("s");
    else if (S_ISCHR(mode)) printf("c");
    else if (S_ISBLK(mode)) printf("b");
    else printf("-");
    
    printf(mode & S_IRUSR ? "r" : "-");
    printf(mode & S_IWUSR ? "w" : "-");
    printf(mode & S_IXUSR ? "x" : "-");
    
    printf(mode & S_IRGRP ? "r" : "-");
    printf(mode & S_IWGRP ? "w" : "-");
    printf(mode & S_IXGRP ? "x" : "-");
    
    printf(mode & S_IROTH ? "r" : "-");
    printf(mode & S_IWOTH ? "w" : "-");
    printf(mode & S_IXOTH ? "x" : "-");
}

void printDetailedListing(const char* dirPath, struct dirent** entryList, int numEntries) 
{
    printf("Permissions %-2s %-8s %-8s %8s %s\t       %s\n", "L#", "Owner", "Group", "Size", "Date", "Name");
    for(int i = 0; i < numEntries; i++) 
    {
        char fullPath[PATH_MAX];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", dirPath, entryList[i]->d_name);
        
        struct stat fileStat;
        if (lstat(fullPath, &fileStat) == -1) 
        {
            fprintf(stderr, "stat: %s: No such file or directory\n", fullPath);
            continue;
        }
        
        printFilePermissions(fileStat.st_mode);
        printf(" ");
        printf("%3ld ", (long)fileStat.st_nlink);
        
        struct passwd *pw = getpwuid(fileStat.st_uid);
        if (pw) printf("%-8s ", pw->pw_name);
        else printf("%-8d ", (int)fileStat.st_uid);
        
        struct group *gr = getgrgid(fileStat.st_gid);
        if (gr) printf("%-8s ", gr->gr_name);
        else printf("%-8d ", (int)fileStat.st_gid);
        
        // File size
        if (S_ISCHR(fileStat.st_mode) || S_ISBLK(fileStat.st_mode)) 
        {
            printf("%4d,%4d ", major(fileStat.st_rdev), minor(fileStat.st_rdev));
        } 
        else 
        {
            printf("%8ld ", (long)fileStat.st_size);
        }
        char timeStr[32];
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&fileStat.st_mtime);
        
        if (now - fileStat.st_mtime > 6 * 30 * 24 * 60 * 60) 
        {
            strftime(timeStr, sizeof(timeStr), "%b %d  %Y", tm_info);
        } 
        else 
        {
            strftime(timeStr, sizeof(timeStr), "%b %d %H:%M", tm_info);
        }
        printf("%s ", timeStr);
        printf("%s", entryList[i]->d_name);
        
        if (S_ISLNK(fileStat.st_mode)) 
        {
            char linkTarget[PATH_MAX];
            ssize_t len = readlink(fullPath, linkTarget, sizeof(linkTarget) - 1);
            if (len != -1) 
            {
                linkTarget[len] = '\0';
                printf(" -> %s", linkTarget);
            }
        }
        
        printf("\n");
    }
}

int lsBuiltin(char **args) 
{
    struct dirent **entryList;
    int numEntries;
    char* dir = ".";
    int detailed = 0;
    

    if(args[1] != NULL) // Arguments
    {
        if(args[2] != NULL) //Path specified w -a/-la
        {
            dir = args[2];
            int (*filter)(const struct dirent*) = NULL;
            if(strcmp(args[1], "-a") && strcmp(args[1], "-la")) filter = lsaFilter; // If not specified, don't check for hidden files
            else if(!strcmp(args[1], "-la")) detailed = 1; // Checks for hidden files by default
            numEntries = scandir(dir, &entryList, filter, NULL);
        }
        else if(args[1][0] != '-') //Path specified w/o -a/-la
        {
            dir = args[1];
            numEntries = scandir(dir, &entryList, lsaFilter, NULL);
        }
        else if(!strcmp(args[1], "-a"))
        {
            numEntries = scandir(dir, &entryList, NULL, NULL);
        }
        else if(!strcmp(args[1], "-la"))
        {
            detailed = 1;
            numEntries = scandir(dir, &entryList, NULL, NULL);
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

    if (detailed) 
    {
        printDetailedListing(dir, entryList, numEntries);
    } else {
        for(int i = 0; i < numEntries; i++)
        {
            printf("%s  ", entryList[i]->d_name);
        }
        printf("\n");
    }

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
