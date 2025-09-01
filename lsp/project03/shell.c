#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>

// str 에서 앞뒤 공백 무시하고 int 형 정수 하나 읽기, 그외 문자 존재시 fallback 반환
int parseExactInt(char *str, int fallback) {
    while (*str != '\0') {
        if (!isspace(*str)) {
            break;
        }
        str++;
    }

    errno = 0;
    char *endptr;
    const int ret = (int) strtol(str, &endptr, 10);

    while (*endptr != '\0') {
        if (!isspace(*endptr)) {
            return fallback;
        }
        endptr++;
    }

    if (errno == 0 && str != endptr) {
        return ret;
    }
    return fallback;
}

// 쉘 프롬프트 출력
void shellPrompt(char *buffer, int size) {
    printf(SHELL_PROMPT);
    fgets(buffer, size, stdin);
    buffer[strlen(buffer) - 1] = '\0';
}

// 라인 단위로 입력한 문자열을 del를 기준으로 잘라서 반환하는 함수
char **divideLine(char *str, int *argc, char *del) {
    *argc = 0; char *temp_list[100] = {(char *)NULL, };
    char *token = NULL;

    token = strtok(str, del);
    if (token == NULL) return NULL;
    while (token != NULL) {
        temp_list[(*argc)++] = token;
        token = strtok(NULL, del);
    }

    char **argv = (char **)malloc(sizeof(char *) * (*argc + 1));
    for (int i = 0; i < *argc; i++) {
        argv[i] = temp_list[i];
    }
    argv[*argc] = NULL;

    return argv;
}