#include "modify.h"
#include "daemonconfig.h"
#include "daemonlist.h"
#include "shell.h"
#include "file.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <linux/limits.h>
#include <string.h>

void printModifyUsage() {
    printf("Usage: modify <DIR_PATH> [OPTIONS]...\n");
}

void execModify(char *input) {
    bool result;
    DaemonConfig config;
    initDaemonConfig(&config);

    // args 파싱
    int argc;
    char *args[1024];
    result = lineToArgs(input, &argc, args, 256);
    if (!result || argc < 1) {
        printModifyUsage();
        return;
    }

    // <DIR_PATH> 파싱
    if (!validateDaemonDirPath(args[0], config.dirPath)) {
        return;
    }

    // 이미 모니터링중인 디렉토리가 맞는지 확인
    FILE *file = openCurrentDaemonListFile("r+");
    if (file == NULL) {
        printf("failed to open current daemon list file\n");
        return;
    }

    DaemonList *list = readCurrentDaemonList(file);
    DaemonList *current = findDaemonListItemByDirPath(list, config.dirPath);
    bool isMonitored = (current != NULL);
    freeCurrentDaemonList(list);
    closeCurrentDaemonListFile(file);

    if (!isMonitored) {
        printf("error: directory is not being monitored, %s\n", config.dirPath);
        return;
    }

    // 기존 옵션 읽기
    readDaemonConfigFile(&config);

    // [OPTIONS] 파싱
    result = parseDaemonConfigOptions(&config, argc - 1, args + 1);
    if (!result) {
        printModifyUsage();
        return;
    }

    // 이외 유효성 검사
    result = validateDaemonConfig(&config);
    if (!result) {
        return;
    }

    // config 저장
    writeDaemonConfigFile(&config);
}
