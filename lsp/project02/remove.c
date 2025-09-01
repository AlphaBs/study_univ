#include "remove.h"
#include "daemonconfig.h"
#include "daemonlist.h"
#include "shell.h"
#include "file.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

void printUsage() {
    printf("Usage: remove <DIR_PATH>\n");
}

void execRemove(char *args) {
    // <DIR_PATH> 파싱
    char dirPath[PATH_MAX];
    if (!validateDaemonDirPath(args, dirPath)) {
        return;
    }

    // 이미 모니터링중인 디렉토리가 맞는지 확인
    FILE *file = openCurrentDaemonListFile("r+");
    if (file == NULL) {
        printf("failed to open current daemon list file\n");
        return;
    }

    DaemonList *list = readCurrentDaemonList(file);
    bool isDeleted = deleteDaemonListItemByDirPath(list, dirPath);
    closeCurrentDaemonListFile(file);

    if (isDeleted) {
        // 데몬 config 읽기
        DaemonConfig config;
        initDaemonConfig(&config);
        strcpy(config.dirPath, dirPath);
        readDaemonConfigFile(&config);

        // kill process
        if (kill(config.pid, SIGTERM) != 0) {
            printf("error: failed to kill process, %s\n", dirPath);
        }

        // 리스트 저장
        file = openCurrentDaemonListFile("w+");
        if (file == NULL) {
            printf("failed to open current daemon list file\n");
        } else {
            writeCurrentDaemonList(file, list);
            closeCurrentDaemonListFile(file);
        }
    } else {
        printf("error: directory is not being monitored, %s\n", dirPath);
    }

    freeCurrentDaemonList(list);
}
