#ifndef CASH_H
#define CASH_H

#define CMD_BUFFSIZE 1024
#define MAXARGS 10
#define WHITESPACE ' '

void cash_loop(void);
int splitCommandLine(char *commandBuffer, char* args[]);
char *skipChar(char *charPtr, char skip);
char *expandTilde(char* path);
char* reduceTilde(char* path);

#endif // CASH_H