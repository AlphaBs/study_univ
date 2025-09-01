#include "help.h"
#include <stdio.h>
#include <string.h>

void helpTree()
{
    printf(
        "Usage: \n"
        "  > tree <PATH> [OPTION]... : display the directory structure if <PATH> is a directory\n"
        "    -r : display the directory structure recursively if <PATH> is a directory\n"
        "    -s : display the directory structure if <PATH> is a directory, including the size of each file\n"
        "    -p : display the directory structure if <PATH> is a directory, including the permissions of each directory and file\n");
}

void helpPrint()
{
    printf(
        "Usage: \n"
        "  > print <PATH> [OPTION]... : print the contents on the standard output if <PATH> is file\n"
        "    -n <line_number> : print only the first <line_number> lines of its contents on the standard output if <PATH> is file\n");
}

void helpHelp()
{
    printf(
        "Usage: \n"
        " > help [COMMAND] : show commands for program\n");
}

void helpExit()
{
    printf(
        "Usage: \n"
        " > exit : exit program\n");
}

void helpAll()
{
    printf(
        "Usage: \n"
        "  > tree <PATH> [OPTION]... : display the directory structure if <PATH> is a directory\n"
        "    -r : display the directory structure recursively if <PATH> is a directory\n"
        "    -s : display the directory structure if <PATH> is a directory, including the size of each file\n"
        "    -p : display the directory structure if <PATH> is a directory, including the permissions of each directory and file\n"
        "  > print <PATH> [OPTION]... : print the contents on the standard output if <PATH> is file\n"
        "    -n <line_number> : print only the first <line_number> lines of its contents on the standard output if <PATH> is file\n"
        " > help [COMMAND] : show commands for program\n"
        " > exit : exit program\n");
}

void execHelp(int argc, char **argv)
{
    if (argc == 1)
    {
        helpAll();
    }
    else if (argc == 2)
    {
        if (strcmp(argv[1], "tree") == 0)
        {
            helpTree();
        }
        else if (strcmp(argv[1], "print") == 0)
        {
            helpPrint();
        }
        else if (strcmp(argv[1], "help") == 0)
        {
            helpHelp();
        }
        else if (strcmp(argv[1], "exit") == 0)
        {
            helpExit();
        }
        else
        {
            printf("invalid command -- '%s'\n", argv[1]);
            helpAll();
        }
    }
    else
    {
        printf("invalid command -- '%s'\n", argv[1]);
        helpAll();
    }
}