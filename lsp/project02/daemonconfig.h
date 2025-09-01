#ifndef DAEMONCONFIG_H
#define DAEMONCONFIG_H

#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include "linux/limits.h"

#define DAEMON_CONFIG_FILE_NAME "ssu_cleanupd.config"
#define MAX_EXCLUDE_PATH_COUNT 1024
#define MAX_EXTENSION_COUNT 1024
#define DUPLICATED_FILE_MODE_LATEST 1
#define DUPLICATED_FILE_MODE_OLDEST 2
#define DUPLICATED_FILE_MODE_IGNORE 3

typedef struct DaemonConfig
{
    char dirPath[PATH_MAX];
    char outputPath[PATH_MAX];
    int timeInterval;
    int maxLogLines;
    char *excludePathList[MAX_EXCLUDE_PATH_COUNT];
    int excludePathCount;
    char *extensionList[MAX_EXTENSION_COUNT];
    int extensionCount;
    int duplicatedFileMode;
    int pid;
    time_t startTime;
} DaemonConfig;

void initDaemonConfig(DaemonConfig* config);
bool validateDaemonDirPath(char* dirPath, char* realDirPath);
bool validateDaemonConfig(DaemonConfig* config);
bool parseDaemonConfigOptions(DaemonConfig* config, int argc, char* args[]);
void freeDaemonConfigItems(DaemonConfig* config);
void getDaemonConfigFilePath(DaemonConfig* config, char* path);
FILE* openDaemonConfigFile(DaemonConfig* config, const char* mode);
void closeDaemonConfigFile(FILE* file);
void readDaemonConfigFile(DaemonConfig* config);
void writeDaemonConfigFile(DaemonConfig* config);

#endif
