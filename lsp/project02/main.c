#include "daemon.h"
#include "shell.h"
#include "show.h"
#include "add.h"
#include "modify.h"
#include "remove.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// help 명령어 처리
void help() {
    printf(
        "Usage:\n"
        "> show\n"
        "  <none> : show monitoring daemon process info\n"
        "> add <DIR_PATH> [OPTION]...\n"
        "  <none> : add daemon process monitoring the <DIR_PATH> directory\n"
        "  -d <OUTPUT_PATH> : Specify the output directory <OUTPUT_PATH> where <DIR_PATH> will be arranged\n"
        "  -i <TIME_INTERVAL> : Set the time interval for the daemon process to monitor in seconds.\n"
        "  -l <MAX_LOG_LINES> : Set the maximum number of log lines the daemon process will record.\n"
        "  -x <EXCLUDE_PATH1, EXCLUDE_PATH2, ...> : Exclude all subfiles in the specified directories.\n"
        "  -e <EXTENSION1, EXTENSION2, ...> : Specify the file extensions to be organized.\n"
        "  -m <M> : Specify the value for the <M> option.\n"
        "> modify <DIR_PATH> [OPTION]...\n"
        "  <none> : modify daemon process config monitoring the <DIR_PATH> directory\n"
        "  -d <OUTPUT_PATH> : Specify the output directory <OUTPUT_PATH> where <DIR_PATH> will be arranged\n"
        "  -i <TIME_INTERVAL> : Set the time interval for the daemon process to monitor in seconds.\n"
        "  -l <MAX_LOG_LINES> : Set the maximum number of log lines the daemon process will record.\n"
        "  -x <EXCLUDE_PATH1, EXCLUDE_PATH2, ...> : Exclude all subfiles in the specified directories.\n"
        "  -e <EXTENSION1, EXTENSION2, ...> : Specify the file extensions to be organized.\n"
        "  -m <M> : Specify the value for the <M> option.\n"
        "> remove <DIR_PATH>\n"
        "  <none> : remove daemon process monitoring the <DIR_PATH> directory\n"
        "> help\n"
        "> exit\n"
    );
}

int main() {
    // 데몬 초기화
    initDaemonDir();

    // 쉘 루프
    char input[11111];
    while (true) {
        // 입력 받기
        int result = shellPrompt(input, 11111);
        if (result == EOF) // EOF 발견 시 프로그램 종료
        {
            return 0;
        }

        // 입력 문자열에서 처음으로 마주치는 공백 문자를 NULL 문자로 치환
        char *next = strcut(input, ' ');

        // 아무 입력도 없는 경우, 다시 입력 받기
        if (strcmp(input, "") == 0) {
            continue;
        } else if (strcmp(input, "exit") == 0) {
            return 0;
        } else if (strcmp(input, "show") == 0) {
            execShow(next);
        } else if (strcmp(input, "add") == 0) {
            execAdd(next);
        } else if (strcmp(input, "modify") == 0) {
            execModify(next);
        } else if (strcmp(input, "remove") == 0) {
            execRemove(next);
        } else if (strcmp(input, "help") == 0) {
            help();
        } else {
            help();
        }
    }
    return 0;
}