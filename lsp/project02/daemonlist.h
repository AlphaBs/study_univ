#ifndef DAEMONLIST_H
#define DAEMONLIST_H

#include <stdio.h>
#include <stdbool.h>

#define CURRENT_DAEMON_LIST_FILE_NAME "current_daemon_list"

typedef struct DaemonList DaemonList;
struct DaemonList
{
    char* dirPath;
    DaemonList* next;
};

void getCurrentDaemonListFilePath(char* path);
FILE* openCurrentDaemonListFile(const char* mode);
int readPathCurrentDaemonList(FILE* file, char* buf);
DaemonList* readCurrentDaemonList(FILE* file);
void writeCurrentDaemonList(FILE* file, DaemonList* list);
void freeCurrentDaemonList(DaemonList* list);
void closeCurrentDaemonListFile(FILE* file);

DaemonList *findDaemonListItemByIndex(DaemonList *list, int index);
DaemonList *findDaemonListItemByDirPath(DaemonList *list, char *dirPath);
void addDaemonListItem(DaemonList *list, char *dirPath);
bool deleteDaemonListItemByDirPath(DaemonList *list, char *dirPath);

#endif
