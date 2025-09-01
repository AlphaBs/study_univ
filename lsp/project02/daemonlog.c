#include "daemonlog.h"
#include "shell.h"

#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

// <DIR_PATH>/ssu_cleanupd.log 경로 만들기
void getDaemonLogFilePath(const DaemonConfig *config, char *path) {
    strcpy(path, config->dirPath);
    strcat(path, "/");
    strcat(path, DAEMON_LOG_FILE_NAME);
}

// 로그 파일이 없다면 빈 파일 생성
void ensureDaemonLogFile(const DaemonConfig *config) {
    char path[PATH_MAX];
    getDaemonLogFilePath(config, path);

    if (access(path, F_OK) == -1 && errno == ENOENT) {
        FILE* fp = fopen(path, "w");
        if (fp != NULL) {
            fclose(fp);
        }
    }
}

// 로그 파일 열기
FILE *openDaemonLogFile(const DaemonConfig *config, const char *mode) {
    char path[PATH_MAX];
    getDaemonLogFilePath(config, path);

    FILE *file = fopen(path, mode);
    return file;
}

// 로그 파일 읽기
DaemonLogList *readDaemonLogFile(FILE *file) {
    rewind(file);

    char buf[DAEMON_LOG_LINE_MAX_LENGTH];
    char trimmed[DAEMON_LOG_LINE_MAX_LENGTH];

    // 링크드리스트 시작점 초기화
    DaemonLogList *list = malloc(sizeof(DaemonLogList));
    list->item.line = NULL;
    list->item.length = 0;
    list->next = NULL;

    // 한줄씩 읽으면서 리스트에 추가
    DaemonLogList *prev = list;
    while (fgets(buf, DAEMON_LOG_LINE_MAX_LENGTH, file) != NULL) {
        trim(trimmed, buf); // 빈 라인 무시
        if (trimmed[0] == '\0') {
            continue;
        }

        // 새로운 노드 추가
        DaemonLogList *item = malloc(sizeof(DaemonLogList));
        item->item.line = strdup(trimmed);
        item->next = NULL;

        prev->next = item;
        prev = item;
        list->item.length++;
    }

    return list;
}

// 로그 내용을 파일에 쓰기
void writeDaemonLogFile(FILE *file, const DaemonLogList *list) {
    rewind(file);

    const DaemonLogList *current = list->next;
    while (current != NULL) {
        fprintf(file, "%s\n", current->item.line);
        current = current->next;
    }
}

// 로그를 저장하는 링크드리스트 메모리 해제
void freeDaemonLogList(DaemonLogList *list) {
    DaemonLogList *current = list->next;
    while (current != NULL) {
        free(current->item.line);
        DaemonLogList *next = current->next;
        free(current);
        current = next;
    }

    free(list);
}

// 로그 포맷 만들기
void formatDaemonLog(char *buf, const DaemonConfig *config, time_t timestamp, char *srcPath, char *destPath) {
    struct tm *localTime = localtime(&timestamp);

    snprintf(buf, DAEMON_LOG_LINE_MAX_LENGTH,
             "[%02d:%02d:%02d][%d][%s][%s]",
             localTime->tm_hour,
             localTime->tm_min,
             localTime->tm_sec,
             config->pid,
             srcPath,
             destPath);
}

// 로그 리스트에 새로운 로그 추가
void appendDaemonLog(DaemonLogList *list, const DaemonConfig *config, time_t timestamp, char *srcPath, char *destPath) {
    // 새로운 로그 문자열 만들기
    char buf[DAEMON_LOG_LINE_MAX_LENGTH];
    formatDaemonLog(buf, config, timestamp, srcPath, destPath);

    // maxLogLines 가 설정된 경우 오래된 로그 제거
    while (config->maxLogLines >= 0 && list->item.length >= config->maxLogLines) {
        DaemonLogList *first = list->next;
        DaemonLogList *second = first->next;

        free(first->item.line);
        free(first);

        list->next = second;
        list->item.length--;
    }

    // 새로운 노드 할당
    DaemonLogList *item = malloc(sizeof(DaemonLogList));
    item->item.line = strdup(buf);
    item->next = NULL;

    // 마지막 노드 찾기
    DaemonLogList *current = list;
    while (current->next != NULL) {
        current = current->next;
    }

    // 리스트에 노드 추가
    current->next = item;
    list->item.length++;
}
