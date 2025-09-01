#include "show.h"

#include <assert.h>

#include "daemon.h"
#include "daemonlist.h"
#include "daemonconfig.h"
#include "daemonlog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include "shell.h"

void execShow(char *args) {
    (void) args;

    // 디몬 리스트 파일 열기
    FILE *file = openCurrentDaemonListFile("r+");
    if (file == NULL) {
        printf("error: openCurrentDaemonListFile\n");
        return;
    }

    printf(
        "Current working daemon process list\n\n"
        "0. exit\n");

    // 리스트 읽고 화면에 출력
    DaemonList *list = readCurrentDaemonList(file);
    DaemonList *current = list->next;
    int number = 1;
    while (current != NULL) {
        printf("%d. %s\n", number, current->dirPath);
        current = current->next;
        number++;
    }

    putchar('\n');

    int select = 0;
    char buffer[64];
    while (true) {
        printf("Select one to see process info : "); // 번호 입력받기
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            buffer[strlen(buffer) - 1] = '\0';
            select = parseExactInt(buffer, -1);
            if (select == 0)
                return;

            current = findDaemonListItemByIndex(list, select - 1); // 번호에 해당하는 노드 찾기

            if (current == NULL) {
                printf("Please check your input is valid\n");
                continue;
            }
        }
        break;
    }

    assert(current != NULL);

    // 디몬 설정 출력
    DaemonConfig config;
    strcpy(config.dirPath, current->dirPath);

    printf("\n%d. config detail\n\n", select);

    // 설정파일 열기
    FILE *configFile = openDaemonConfigFile(&config, "r+");
    if (configFile == NULL) {
        printf("error: openDaemonConfigFile\n");
        return;
    }

    // 파일 내용 그대로 출력
    int c;
    while ((c = fgetc(configFile)) != EOF) {
        putchar(c);
    }
    closeDaemonConfigFile(configFile);

    // 로그 출력
    printf("\n2. log detail\n\n");

    // 로그파일 열기
    FILE *logFile = openDaemonLogFile(&config, "r");
    if (logFile == NULL) {
        // no file
        return;
    }

    int lineIndex = 0;
    int lineCount = 0;
    int logLength = 0;
    char lines[10][PATH_MAX * 2 + 64];

    // 로그 파일 읽기
    while (true) {
        const int ch = fgetc(logFile);
        if (ch == EOF) {
            break;
        }

        if (ch == '\n') { // 라인 다 읽었으면 다음 라인으로 넘어가기
            lines[lineIndex][logLength] = '\0';
            logLength = 0;
            lineIndex = (lineIndex + 1) % 10;
            lineCount++;
        } else { // 라인 끝까지 배열에 저장
            lines[lineIndex][logLength] = (char) ch;
            logLength++;
        }
    }

    // 로그 라인이 저장된 lines 배열에서 어디서부터 어디까지 출력할지 결정
    int loopStart = 0;
    int loopCount = lineCount;

    if (loopCount > 10) {
        loopStart = lineIndex;
        loopCount = 10;
    }

    // 화면에 출력
    for (int i = 0; i < loopCount; i++) {
        int line = (loopStart + i) % 10;
        printf("%s\n", lines[line]);
    }

    freeCurrentDaemonList(list);
    closeCurrentDaemonListFile(file);

    printf("\n");
}
