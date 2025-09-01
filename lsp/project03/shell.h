#ifndef SHELL_H
#define SHELL_H

#define SHELL_PROMPT "20211400>"

int parseExactInt(char *str, int fallback);
void shellPrompt(char *buffer, int size);
char **divideLine(char *str, int *argc, char *del);

#endif