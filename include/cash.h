#ifndef CASH_H
#define CASH_H

#define CMD_BUFFSIZE 1024
#define MAXARGS 10
#define WHITESPACE ' '

void cashLoop(void);
int splitCommandLine(char *commandBuffer, char* args[]);
char *skipChar(char *charPtr, char skip);
char *expandTilde(char* path);
char* reduceTilde(char* path);
int runCommand(char *args[]);
void disableISIG(void);
void enableISIG(void);

#endif // CASH_H