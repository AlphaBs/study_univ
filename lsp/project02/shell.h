#ifndef SHELL_H
#define SHELL_H

#include <stdbool.h>
#include <sys/types.h>

#define SHELL_PROMPT "20211400>"

void trim(char *result, char *str);
int parseExactInt(char *str, int fallback);
char* strcut(char *input, char value);
int shellPrompt(char *buffer, int size);
bool lineToArgs(char* buffer, int *argc, char *argv[], int maxArgc);

#endif