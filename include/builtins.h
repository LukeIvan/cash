#ifndef BUILTINS_H
#define BUILTINS_H

#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#define UNUSED(x) ((void)(x))

typedef int (*builtinFunction)(char *args[]);

typedef struct {
    const char *name;
    const char *description;
    const char *usage;
    builtinFunction builtinFunction;
} builtin;

int cdBuiltin(char **args);
int lsBuiltin(char **args);
int helpBuiltin(char **args);
int pwdBuiltin(char **args);
int aliasBuiltin(char **args);
int exportBuiltin(char **args);
int exitBuiltin(char **args);

void printFilePermissions(mode_t mode);
void printDetailedListing(const char* dirPath, struct dirent** entryList, int numEntries);

/*
    TODO: UPDATE UNFINISHED DESCRIPTIONS
*/

static builtin builtinList[] = {
    {"cd", "Change the current directory", "cd <directory>", cdBuiltin},
    {"ls", "List directory contents UNFINISHED", "ls [-a|-la] <directory>", lsBuiltin},
    {"pwd", "Print the current working directory", "pwd", pwdBuiltin},
    {"export", "Set environment variables UNFINISHED", "export VAR=value", exportBuiltin},
    {"alias", "Create command aliases UNFINISHED", "alias name=command", aliasBuiltin},
    {"help", "Display help information", "help command", helpBuiltin},
    {"exit", "Exit the shell", "exit", exitBuiltin},
    {NULL, NULL, NULL, NULL}
};

int runBuiltinCommand(char *args[]);

#endif // BUILTINS_h