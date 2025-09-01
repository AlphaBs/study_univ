#define _GNU_SOURCE

#include "shell.h"
#include "debug.h"
#include "file.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int directoryCount;
int fileCount;

// dirPath/subPath 경로 생성
char *concatPath(const char *dirPath, const char *subPath)
{
    int baseLength = strlen(dirPath);
    int subLength = strlen(subPath);
    char *concatPath = malloc(baseLength + subLength + 2);
    for (int i = 0; i < baseLength; i++)
    {
        concatPath[i] = dirPath[i];
    }
    concatPath[baseLength] = '/';
    for (int i = 0; i < subLength; i++)
    {
        concatPath[baseLength + 1 + i] = subPath[i];
    }
    concatPath[baseLength + subLength + 1] = '\0';
    return concatPath;
}

// 노드 정보 출력
void printNodeStat(const char *path, bool showSize, bool showPermission) 
{
    struct stat statbuf;
    if (stat(path, &statbuf) == -1) 
    {
        printf("error: %d\n", errno);
        return;
    }

    if (showPermission && showSize)
    {
        putchar('[');
        printPermission(statbuf.st_mode);
        printf(" %ld] ", statbuf.st_size);
    }
    else if (showPermission)
    {
        putchar('[');
        printPermission(statbuf.st_mode);
        putchar(']');
        putchar(' ');
    }
    else if (showSize)
    {
        printf("[%ld] ", statbuf.st_size);
    }
}

// 트리 재귀 출력
bool printTree(const char *basePath, bool showSize, bool showPermission, int depth, bool *tree)
{
    struct dirent **namelist;
    int n;

    n = scandir(basePath, &namelist, NULL, alphasort);
    if (n == -1)
    {
        if (errno != 20)
            printf("error: %d\n", errno);
        return false;
    }

    for (int i = 0; i < n; i++) 
    {
        if (strcmp(namelist[i]->d_name, ".") != 0 && strcmp(namelist[i]->d_name, "..") != 0) 
        {
            // 트리 선 그리기
            for (int d = 0; d < depth; d++)
            {
                if (tree[d])
                {
                    printf("│   ");
                }
                else
                {
                    printf("    ");
                }
            } 

            if (i == n - 1) // 마지막 노드면 ㄴ 선
            {
                printf("└─ ");
                tree[depth] = false;
            }
            else // 마지막 노드가 아니라면 ㅏ 선
            {
                printf("├─ ");
                tree[depth] = true;
            }

            // 현재 노드의 전체 경로 생성
            char *fullPath = concatPath(basePath, namelist[i]->d_name);

            if (namelist[i]->d_type == DT_DIR) 
            {
                directoryCount++;
                // 파일 크기, 접근 권한 출력
                printNodeStat(fullPath, showSize, showPermission);
                printf("%s/\n", namelist[i]->d_name);
                printTree(fullPath, showSize, showPermission, depth + 1, tree);
            }
            else
            {
                fileCount++;
                // 파일 크기, 접근 권한 출력
                printNodeStat(fullPath, showSize, showPermission);
                printf("%s\n", namelist[i]->d_name);
            }

            free(fullPath);
        }
        free(namelist[i]);
    }

    free(namelist);
    return true;
}

// 명령어 실행
bool execTree(char *input)
{
    bool result;

    const char *dirPath = 0;
    bool showSize = false;
    bool showPermission = false;

    // 명령어 파싱
    int argc;
    char *args[4];
    result = lineToArgs(input, &argc, args, 4);
    if (!result || argc == 0)
    {
        return false;
    }

    dirPath = args[0];
    if (strlen(dirPath) > PATH_MAX)
    {
        printf("error: too long path\n");
        return true;
    }

    for (int i = 1; i < argc; i++)
    {
        if (args[i][0] == '-' && args[i][1] != '\0')
        {
            for (int j = 1; args[i][j] != '\0'; j++)
            {
                if (args[i][j] == 's')
                {
                    showSize = true;
                }
                else if (args[i][j] == 'p')
                {
                    showPermission = true;
                }
                else
                {
                    return false;   
                }
            }
        }
        else
        {
            return false;
        }
    }

    // 트리 초기화
    bool tree[PATH_MAX];
    for (int d = 0; d < PATH_MAX; d++)
    {
        tree[d] = false;
    }

    // 상대 경로를 절대 경로로 변환
    char* realPath = getShellRealPath(dirPath);
    if (realPath == NULL)
    {
        // 잘못된 경로 혹은 존재하지 않는 경로
        return false;
    }

    // 절대 경로가 홈 디렉토리를 벗어나는지 확인
    char* homeDir = getHomeDir();
    int homeDirLength = strlen(homeDir);
    if (strncmp(realPath, homeDir, homeDirLength) != 0) 
    {
        printf("%s is outside the home directory\n", dirPath);
        free(realPath);
        return true;
    }

    // 존재하는지 확인
    struct stat dirStat;
    if (stat(realPath, &dirStat) != 0)
    {
        free(realPath);
        return false;
    }

    // 디렉토리가 맞는지 확인
    if (!S_ISDIR(dirStat.st_mode))
    {
        printf("%s is not a directory\n", dirPath);
        free(realPath);
        return true;
    }

#if DEBUG
    printf("dirPath: %s\n", dirPath);
    printf("realPath: %s\n", realPath);
    printf("showSize: %d\n", showSize);
    printf("showPermission: %d\n", showPermission);
#endif

    // 디렉토리 개수, 파일 개수 초기화
    directoryCount = 1; // 시작 디렉토리 포함
    fileCount = 0;

    // 트리 출력
    printNodeStat(realPath, showSize, showPermission);
    printf("%s\n", dirPath);
    result = printTree(realPath, showSize, showPermission, 0, tree);
    if (result)
    {
        printf("\n%d directories, %d files\n", directoryCount, fileCount);
    }

    free(realPath);
    return true;
}