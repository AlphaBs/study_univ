#include "add.h"
#include "debug.h"
#include "daemon.h"
#include "daemonconfig.h"
#include "daemonlist.h"
#include "shell.h"
#include "file.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

void printUsageAdd() {
    printf("Usage: add <DIR_PATH> [OPTIONS] ...\n");
}

void execAdd(char *input) {
    // 설정 초기화
    DaemonConfig config;
    initDaemonConfig(&config);

    // args 파싱
    int argc;
    char *args[1024];
    bool result = lineToArgs(input, &argc, args, 256);
    if (!result || argc < 1) { // 인자 없이 호출
        printUsageAdd();
        return;
    }

    // <DIR_PATH> 파싱
    // 경로 유효성 검사하고 절대 경로로 변환
    if (!validateDaemonDirPath(args[0], config.dirPath)) {
        return;
    }

    // 디몬 리스트 열기
    FILE *file = openCurrentDaemonListFile("r+");
    if (file == NULL) {
        printf("failed to open current daemon list file\n");
        return;
    }

    // 디몬 리스트 읽기
    DaemonList *list = readCurrentDaemonList(file);
    DaemonList *current = list->next;

    // 이미 모니터링중인 디렉토리의 상위 혹은 하위 디렉토리인지 확인
    bool isMonitored = false;
    while (current != NULL) {
        if (isSubDir(current->dirPath, config.dirPath) ||
            isSubDir(config.dirPath, current->dirPath)) {
            isMonitored = true;
            break;
        }
        current = current->next;
    }

    if (isMonitored) {
        printf("error: directory is already being monitored, %s\n", config.dirPath);
        goto cleanDaemonList;
    }

    // [OPTIONS] 파싱
    result = parseDaemonConfigOptions(&config, argc - 1, args + 1);
    if (!result) {
        printUsageAdd();
        goto cleanDaemonConfig;
    }

    // <OUTPUT_PATH> 설정되지 않았다면 기본경로 <DIR_PATH>_arranged 으로 설정
    if (config.outputPath[0] == '\0') {
        snprintf(config.outputPath, PATH_MAX + 10, "%s_arranged", config.dirPath);
        mkdir(config.outputPath, 0755);
    }

    // 디몬 설정 유효성 검사
    result = validateDaemonConfig(&config);
    if (!result) {
        goto cleanDaemonConfig;
    }

    // 데몬 프로세스 실행
    forkDaemon(&config);
    //runDaemon(&config); // 디버그용, 데몬이 아닌 현재 프로세스에서 arrange 실행

    // 새로운 데몬 리스트 생성
    addDaemonListItem(list, config.dirPath);
    writeCurrentDaemonList(file, list);

    // 정리
    cleanDaemonConfig:
    freeDaemonConfigItems(&config);

    cleanDaemonList:
    freeCurrentDaemonList(list);
    closeCurrentDaemonListFile(file);
}
