#define _GNU_SOURCE

#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pwd.h>

#include "help.h"
#include "shell.h"
#include "tree.h"
#include "print.h"

char execPath[PATH_MAX];
char extImagePath[PATH_MAX];

// $HOME 환경변수 반환, free 필요 없음, NULL 반환하는 경우 없음
char *getHomeDir() {
    char *path;
    if ((path = getenv("HOME")) == NULL) {
        path = getpwuid(getuid())->pw_dir;
    }
    return path;
}

// ~ 를 $HOME 으로 바꾸어줌
void replaceHomeDir(char *result, const char *path) {
    char *homeDir = getHomeDir();

    if (path[0] == '~') {
        if (path[1] == '\0') { // ~
            strcpy(result, homeDir);
            return;
        } else if (path[1] == '/') { // ~/, ~//, ~/a
            while (*path != '/') path++; // ~ 다음 문자로 이동
            // <homeDir>/<path> 만들기
            snprintf(result, PATH_MAX, "%s%s", homeDir, path);
            return;
        }
    }

    strcpy(result, path);
}

// . .. ~ 문자를 절대경로로 바꾸어줌
void getShellRealPath(char *result, const char *path) {
    replaceHomeDir(result, path);
    char tmp[PATH_MAX];
    realpath(result, tmp);
    strcpy(result, tmp);
}

void execCommand(int argc, char **argv)
{
    pid_t pid;

    // 0: execPath
    // 1: extImagePath
    // 2 부터 argc 만큼 argv 추가
    // 마지막 NULL
    char **extendedArgv = malloc(sizeof(char *) * (2 + argc + 1));
    extendedArgv[0] = execPath;
    extendedArgv[1] = extImagePath;
    for (int i = 0; i < argc; i++)
    {
        extendedArgv[2 + i] = argv[i];
    }
    extendedArgv[2 + argc] = NULL;

    if ((pid = fork()) < 0)
    {
        fprintf(stderr, "fork error\n");
        exit(1);
    }
    else if (pid == 0)
    {
        // 자식프로세스는 실제 명령어를 수행함
        execv(execPath, extendedArgv);
        exit(0);
    }
    else
    {
        // 부모프로세스는 자식 종료까지 대기
        pid = wait(NULL);
    }

    free(extendedArgv);
}

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        fprintf(stderr, "Usage Error : %s <EXT2_IMAGE>\n", argv[0]);
        return 1;
    }
    else if (argc == 2) // 쉘로 실행
    {
        // 경로 예외처리
        if (strlen(argv[1]) > 4096)
        {
            fprintf(stderr, "exceed path limit 4096\n");
            return 1;
        }

        strcpy(execPath, argv[0]);
        getShellRealPath(extImagePath, argv[1]);

        // 인자가 존재하는지 확인
        struct stat statbuf;
        if (stat(extImagePath, &statbuf) != 0) 
        {
            fprintf(stderr, "failed to stat '%s'\n", extImagePath);
            return 1;
        }

        // 쉘 루프
        int argc;
        char **argv = NULL;
        char input[11111];
        while (true)
        {
            // 입력 받기
            shellPrompt(input, 11111);
            if ((argv = divideLine(input, &argc, " \t")) == NULL)
            {
                continue;
            }

            // 명령어 처리
            if (strcmp(input, "exit") == 0)
            {
                exit(0);
            }
            else if (strcmp(input, "tree") == 0)
            {
                execCommand(argc, argv);
            }
            else if (strcmp(input, "print") == 0)
            {
                execCommand(argc, argv);
            }
            else if (strcmp(input, "help") == 0)
            {
                execHelp(argc, argv);
            }
            else
            {
                helpAll();
            }

            free(argv);
        }
    }
    else // 자식 프로세스에서 실제 명령어 수행
    {
        strcpy(execPath, argv[0]);
        strcpy(extImagePath, argv[1]);

        // <EXT2_IMAGE> 파일 열기
        FILE *fp;
        if ((fp = fopen(extImagePath, "r")) == NULL)
        {
            perror("fopen(extImagePath)");
            return -1;
        }

        // 명령어 수행
        if (strcmp(argv[2], "tree") == 0)
        {
            execTree(fp, argc - 3, argv + 3);
        }
        else if (strcmp(argv[2], "print") == 0)
        {
            execPrint(fp, argc - 3, argv + 3);
        }

        fclose(fp);
    }
    return 0;
}