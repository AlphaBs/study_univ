#ifndef DAEMONLOG_H
#define DAEMONLOG_H

#include "daemonconfig.h"
#include <stdio.h>

#define DAEMON_LOG_LINE_MAX_LENGTH PATH_MAX * 2 + 64
#define DAEMON_LOG_FILE_NAME "ssu_cleanupd.log"

typedef union DaemonLogListItem
{
    char* line;
    int length;
} DaemonLogListItem;

typedef struct DaemonLogList DaemonLogList;
struct DaemonLogList
{
    DaemonLogListItem item;
    DaemonLogList* next;
};

void getDaemonLogFilePath(const DaemonConfig* config, char* path);
void ensureDaemonLogFile(const DaemonConfig *config);
FILE* openDaemonLogFile(const DaemonConfig* config, const char* mode);
DaemonLogList* readDaemonLogFile(FILE* file);
void writeDaemonLogFile(FILE* file, const DaemonLogList* list);
void freeDaemonLogList(DaemonLogList* list);
void appendDaemonLog(DaemonLogList *list, const DaemonConfig *config, time_t timestamp, char *srcPath, char *destPath);

#endif
