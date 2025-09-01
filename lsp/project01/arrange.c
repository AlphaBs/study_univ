#define _GNU_SOURCE

#include "shell.h"
#include "arrange.h"
#include "debug.h"
#include "file.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <sys/wait.h>

#define MAX_EXCLUDE_PATH_COUNT 256
#define MAX_EXTENSION_COUNT 256

#define SELECT_NONE 0
#define SELECT_1 1
#define SELECT_2 2
#define SELECT_BOTH 3

char *arrangeBaseDir;

// 유효성 검사:
// 경로 길이가 4096 초과
// 존재하지 않는 디렉토리
// 디렉토리가 아님
// 홈 디렉토리를 벗어남
bool validateDir(const char *dirPath)
{
    // PATH_MAX 초과 확인
    if (strlen(dirPath) > PATH_MAX)
    {
        printf("error: too long path\n");
        return false;
    }

    char *fullPath = replaceHomeDir(dirPath);
    if (fullPath == NULL)
    {
        printf("error: invalid path\n");
        return false;
    }

    // PATH_MAX 초과 확인
    if (strlen(fullPath) > PATH_MAX)
    {
        printf("error: too long path\n");
        free(fullPath);
        return false;
    }

    // 존재하는지 확인
    struct stat dirStat;
    if (stat(fullPath, &dirStat) != 0)
    {
        printf("%s does not exist\n", dirPath);
        free(fullPath);
        return false;
    }

    // 디렉토리가 맞는지 확인
    if (!S_ISDIR(dirStat.st_mode))
    {
        printf("%s is not a directory\n", dirPath);
        free(fullPath);
        return false;
    }

    char *realPath = realpath(fullPath, NULL);
    if (realPath == NULL)
    {
        printf("error: invalid path\n");
        free(fullPath);
        return false;
    }
    free(fullPath);

    // 홈 디렉토리를 벗어나는지 확인
    char *homeDir = getHomeDir();
    int homeDirLength = strlen(homeDir);
    if (strncmp(realPath, homeDir, homeDirLength) != 0)
    {
        printf("%s is outside the home directory\n", dirPath);
        free(realPath);
        return false;
    }

    free(realPath);
    return true;
}

// vi p 명령어 실행
void runVi(const char *p)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        printf("error: fork\n");
        return;
    }

    if (pid == 0)
    {
        // arrangeBaseDir/p 경로 생성
        char *fullPath = malloc(strlen(arrangeBaseDir) + 1 + strlen(p) + 1);
        snprintf(fullPath, PATH_MAX, "%s/%s", arrangeBaseDir, p);

        // vi 실행
        const char *args[] = {"vi", fullPath, NULL};
        execvp("vi", (char *const *)args);
        free(fullPath);
        exit(1);
    }
    else
    {
        wait(NULL);
    }
}

// diff p1 p2 명령어 실행
void runDiff(const char *p1, const char *p2)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        printf("error: fork\n");
        return;
    }

    if (pid == 0)
    {
        // arrangeBaseDir/p1 경로 생성
        char *fullPath1 = malloc(strlen(arrangeBaseDir) + 1 + strlen(p1) + 1);
        snprintf(fullPath1, PATH_MAX, "%s/%s", arrangeBaseDir, p1);

        // arrangeBaseDir/p2 경로 생성
        char *fullPath2 = malloc(strlen(arrangeBaseDir) + 1 + strlen(p2) + 1);
        snprintf(fullPath2, PATH_MAX, "%s/%s", arrangeBaseDir, p2);

        // diff 실행
        const char *args[] = {"diff", fullPath1, fullPath2, NULL};
        execvp("diff", (char *const *)args);

        free(fullPath1);
        free(fullPath2);
        exit(1);
    }
    else
    {
        wait(NULL);
    }
}

// line 을 읽고 SELECT_1, SELECT_2, SELECT_NONE 중 하나를 반환
int selectNum(const char *line)
{
    if (strcmp(line, "1") == 0)
    {
        return SELECT_1;
    }
    else if (strcmp(line, "2") == 0)
    {
        return SELECT_2;
    }
    else
    {
        return SELECT_NONE;
    }
}

// line 을 읽고 SELECT_1 이라면 p1 을, SELECT_2 라면 p2 를, 둘 다 아니라면 NULL 반환
const char *selectPath(const char *line, const char *p1, const char *p2)
{
    if (strcmp(line, "1") == 0)
    {
        return p1;
    }
    else if (strcmp(line, "2") == 0)
    {
        return p2;
    }
    else
    {
        return NULL;
    }
}

// 충돌 해결 방법 선택
int resolveConflict(struct FilePathPair *p1, struct FilePathPair *p2)
{
    while (true)
    {
        printf("1. %s\n", p1->from);
        printf("2. %s\n", p2->from);
        putchar('\n');

        printf(
            "choose an option: \n"
            "0. select [num]\n"
            "1. diff [num1] [num2]\n"
            "2. vi [num]\n"
            "3. do not select\n");

        char line[64];
        shellPrompt(line, 64);
        char *next = strcut(line, ' ');

        if (strcmp(line, "select") == 0)
        {
            int select = selectNum(next);
            if (select != SELECT_NONE)
            {
                return select;
            }
        }
        else if (strcmp(line, "diff") == 0)
        {
            char *next2 = strcut(next, ' ');
            const char *num1 = selectPath(next, p1->from, p2->from);
            const char *num2 = selectPath(next2, p1->from, p2->from);

            if (num1 != NULL && num2 != NULL)
            {
                runDiff(num1, num2);
            }
        }
        else if (strcmp(line, "vi") == 0)
        {
            const char *select = selectPath(next, p1->from, p2->from);
            if (select != NULL)
            {
                runVi(select);
            }
        }
        else if (strcmp(line, "do") == 0 && strcmp(next, "not select") == 0)
        {
            return SELECT_NONE;
        }
    }
}

// 개별 파일 처리
void arrangeFile(
    struct FilePathPair *list,
    const char *subPath,
    long long after,
    const char *excludePathList[], int excludePathCount,
    const char *extensionList[], int extensionCount)
{
    // -x 옵션의 제외 경로 무시
    for (int i = 0; i < excludePathCount; i++)
    {
        size_t l = strlen(excludePathList[i]);
        if (excludePathList[i][l - 1] == '/') // 만약 경로가 / 로 끝난다면, 디렉토리를 나타내는 완전한 경로
        {
            if (strncmp(excludePathList[i], subPath, l) == 0)
            {
                return;
            }
        }
        else // 디렉토리를 나타내는 완전한 경로가 아니라면 / 를 끝에 추가해서 비교
        {
            if (strncmp(excludePathList[i], subPath, l) == 0)
            {
                if (strlen(subPath) > l && subPath[l] == '/')
                {
                    return;
                }
            }
        }
    }

    // 파일 이름에서 마지막 점의 위치 찾기
    int fullPathLength = strlen(subPath);
    int lastDotIndex = -1;
    for (int j = fullPathLength - 1; j >= 0; j--)
    {
        if (subPath[j] == '.')
        {
            lastDotIndex = j;
            break;
        }
    }

    // 파일 이름에서 마지막 / 위치 찾기
    int lastSlashIndex = -1;
    for (int j = fullPathLength - 1; j >= 0; j--)
    {
        if (subPath[j] == '/')
        {
            lastSlashIndex = j;
            break;
        }
    }

#if DEBUG
    printf("arrangeFile: %s\n", subPath);
#endif

    // 확장자 없는 파일 무시
    int extLength = fullPathLength - lastDotIndex - 1;
    if (extLength <= 0)
    {
        return;
    }

    // 확장자 목록에 포함되어 있는지 확인
    bool isIncluded = false;
    for (int j = 0; j < extensionCount; j++)
    {
        if (strcmp(extensionList[j], subPath + lastDotIndex + 1) == 0)
        {
            isIncluded = true;
            break;
        }
    }

    // 확장자 목록이 존재하면서 목록에 포함되지 않은 파일 무시
    if (!isIncluded && extensionCount > 0)
    {
        return;
    }

    // 파일 절대 경로 생성
    char *fullPath = malloc(strlen(arrangeBaseDir) + 1 + strlen(subPath) + 1);
    snprintf(fullPath, PATH_MAX, "%s/%s", arrangeBaseDir, subPath);

    // 파일 정보 가져오기
    struct stat fileStat;
    int statResult = stat(fullPath, &fileStat);
    free(fullPath);
    if (statResult == -1)
    {
        printf("error: %d\n", errno);
        return;
    }

    // 수정된 시간이 after 이전이라면 무시
    if (fileStat.st_mtime <= after)
    {
        return;
    }

    // { subPath, 확장자/파일이름.확장자 } 생성
    struct FilePathPair *currentPair = malloc(sizeof(struct FilePathPair));
    currentPair->prev = NULL;
    currentPair->next = NULL;

    strcpy(currentPair->from, subPath);
    memset(currentPair->to, 0, sizeof(currentPair->to));

    int fileNameLength = lastDotIndex - lastSlashIndex - 1;
    if (fileNameLength <= 0)
    {
        // . 없는 파일 혹은 . 으로 시작하는 숨김 파일은 _ 에 모으기
        strcat(currentPair->to, "_/");
        strcat(currentPair->to, subPath + lastSlashIndex + 1);
    }
    else
    {
        // 확장자/파일이름.확장자 경로 만들기
        strncpy(currentPair->to, subPath + lastDotIndex + 1, extLength);
        strcat(currentPair->to, "/");
        strncat(currentPair->to, subPath + lastSlashIndex + 1, fileNameLength);
        strcat(currentPair->to, ".");
        strncat(currentPair->to, subPath + lastDotIndex + 1, extLength);
    }

    // 충돌 확인
    int result = SELECT_BOTH;
    struct FilePathPair *conflictPair = NULL;
    while (list != NULL)
    {
        conflictPair = list;

        // 중복 경로가 있는지 확인
        if (strcmp(conflictPair->to, currentPair->to) == 0)
        {
            // 충돌 해결 방법 선택
            result = resolveConflict(conflictPair, currentPair);
            break;
        }
        list = list->next;
    }

    switch (result)
    {
    case SELECT_NONE:
        // 둘 다 버리는 경우, 리스트의 아이템 삭제하고 현재 FilePair 도 삭제
        struct FilePathPair *next = conflictPair->next;
        struct FilePathPair *prev = conflictPair->prev;
        if (prev != NULL)
            prev->next = next;
        if (next != NULL)
            next->prev = prev;
        free(conflictPair);
        free(currentPair);
        break;
    case SELECT_1:
        // 기존 파일 남기는 경우, 현재 FilePair 삭제
        free(currentPair);
        break;
    case SELECT_2:
        // 현재 파일 남기는 경우, conflictPair 를 현재 파일로 교체
        strcpy(conflictPair->from, currentPair->from);
        strcpy(conflictPair->to, currentPair->to);
        free(currentPair);
        break;
    case SELECT_BOTH:
        // 둘 다 유지하는 경우, 리스트 끝에 아이템 추가
        // 이때 conflictPair 는 리스트의 마지막 아이템
        currentPair->prev = conflictPair;
        currentPair->next = NULL;
        conflictPair->next = currentPair;
        break;
    }
}

// 디렉토리 정리
void arrangeDir(
    struct FilePathPair *list,
    const char *subPath,
    long long after,
    const char *excludePathList[], int excludePathCount,
    const char *extensionList[], int extensionCount)
{
    // dirPath/subPath 경로 생성
    char *fullPath = malloc(strlen(arrangeBaseDir) + 1 + strlen(subPath) + 1);
    snprintf(fullPath, PATH_MAX, "%s/%s", arrangeBaseDir, subPath);

#if DEBUG
    printf("arrangeDir: %s\n", fullPath);
#endif

    struct dirent **namelist;
    int n = scandir(fullPath, &namelist, NULL, alphasort);
    free(fullPath);
    if (n == -1)
    {
        printf("error: %d\n", errno);
        return;
    }

    for (int i = 0; i < n; i++)
    {
        if (strcmp(namelist[i]->d_name, ".") != 0 && strcmp(namelist[i]->d_name, "..") != 0)
        {
            // subPath/namelist[i]->d_name 경로 생성
            char *nextPath = malloc(strlen(subPath) + strlen(namelist[i]->d_name) + 2);
            if (strcmp(subPath, "") == 0)
            {
                strcpy(nextPath, namelist[i]->d_name);
            }
            else
            {
                strcpy(nextPath, subPath);
                strcat(nextPath, "/");
                strcat(nextPath, namelist[i]->d_name);
            }

            if (namelist[i]->d_type == DT_DIR)
            {
                arrangeDir(list, nextPath, after, excludePathList, excludePathCount, extensionList, extensionCount);
            }
            else
            {
                arrangeFile(list, nextPath, after, excludePathList, excludePathCount, extensionList, extensionCount);
            }
            free(nextPath);
        }
        free(namelist[i]);
    }

    free(namelist);
}

// 명령어 실행
bool execArrange(char *input)
{
    bool result;

    const char *dirPath = NULL;
    const char *outputPath = NULL;
    long long modifiedSeconds = 0;

    int excludePathCount = 0;
    const char *excludePathList[MAX_EXCLUDE_PATH_COUNT];

    int extensionCount = 0;
    const char *extensionList[MAX_EXTENSION_COUNT];

    // 명령어 파싱
    int argc;
    char *args[256];
    result = lineToArgs(input, &argc, args, 256);
    if (!result || argc < 1) // 잘못된 명령어 혹은 인자가 없을때
    {
        return false;
    }

    dirPath = args[0];

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(args[i], "-d") == 0 && i + 1 < argc)
        {
            if (outputPath != NULL) // 이미 출력 경로가 지정되어 있으면 오류 
            {
                return false;
            }
            if (strlen(args[i + 1]) > PATH_MAX)
            {
                printf("error: too long path\n");
                return false;
            }
            outputPath = args[i + 1];

            i++;
        }
        else if (strcmp(args[i], "-t") == 0 && i + 1 < argc)
        {
            if (modifiedSeconds != 0) // 이미 수정 시간이 지정되어 있으면 오류
            {
                return false;
            }

            errno = 0;
            char *endptr;
            modifiedSeconds = strtoll(args[i + 1], &endptr, 10);
            if (errno == ERANGE || endptr == args[i + 1] || *endptr != '\0') // 숫자가 아닌 경우
            {
                printf("error: invalid -t\n");
                return false;
            }
            if (modifiedSeconds < 0) // 음수인 경우
            {
                printf("error: negative -t\n");
                return false;
            }

            i++;
        }
        else if (strcmp(args[i], "-x") == 0 && i + 1 < argc)
        {
            i++;

            // x 뒤에 오는 모든 인자 읽기
            while (i < argc && excludePathCount < MAX_EXCLUDE_PATH_COUNT && args[i][0] != '-') // x 뒤에 오는 인자들 추가
            {
                if (strlen(args[i]) > PATH_MAX)
                {
                    printf("error: too long path\n");
                    return false;
                }
                excludePathList[excludePathCount++] = args[i++];
            }

            i--; // for 종료후 1 더해지기에 다시 빼줌
        }
        else if (strcmp(args[i], "-e") == 0 && i + 1 < argc)
        {
            i++;

            // e 뒤에 오는 모든 인자 읽기
            while (i < argc && extensionCount < MAX_EXTENSION_COUNT && args[i][0] != '-') // e 뒤에 오는 인자들 추가
            {
                if (strlen(args[i]) > PATH_MAX)
                {
                    printf("error: too long path\n");
                    return false;
                }
                extensionList[extensionCount++] = args[i++];
            }

            i--; // for 종료후에 1 더해지기에 다시 빼줌
        }
        else
        {
            return false;
        }
    }

    // 현재 시간 - modifiedSeconds 계산하여 수정시간이 after 이후 파일만 정리
    long long after = 0;
    if (modifiedSeconds != 0)
    {
        time_t now = time(NULL);
        after = now - modifiedSeconds;
    }

#if DEBUG
    printf("dirPath: %s\n", dirPath);
    printf("outputPath: %s\n", outputPath);
    printf("after: %lld\n", after);
    printf("excludePathCount: %d\n", excludePathCount);
    for (int i = 0; i < excludePathCount; i++)
    {
        printf("excludePathList[%d]: %s\n", i, excludePathList[i]);
    }
    printf("extensionCount: %d\n", extensionCount);
    for (int i = 0; i < extensionCount; i++)
    {
        printf("extensionList[%d]: %s\n", i, extensionList[i]);
    }
#endif

    // 디렉토리 유효성 검사
    if (!validateDir(dirPath))
    {
        return true;
    }

    // ~ 경로 치환
    arrangeBaseDir = replaceHomeDir(dirPath);
    if (arrangeBaseDir == NULL)
    {
        printf("error: invalid path\n");
        return true;
    }

    // 출력 경로를 절대 경로로 변환
    char *realOutputPath = NULL;
    if (outputPath == NULL)
    {
        char *dirName = getDirName(dirPath);
        realOutputPath = malloc(strlen(dirName) + strlen("_arranged") + 1);
        strcpy(realOutputPath, dirName);
        strcat(realOutputPath, "_arranged");
        free(dirName);
    }
    else
    {
        realOutputPath = replaceHomeDir(outputPath);
        if (realOutputPath == NULL)
        {
            printf("error: invalid path\n");
            free(arrangeBaseDir);
            return true;
        }
    }

#if DEBUG
    printf("arrangeBaseDir: %s\n", arrangeBaseDir);
    printf("realOutputPath: %s\n", realOutputPath);
#endif

    // arranged 디렉토리 생성
    mkdir(realOutputPath, 0777);

    // arrange 시작
    const char subPath[1] = {'\0'};

    struct FilePathPair list;
    list.next = NULL;
    list.prev = NULL;
    arrangeDir(&list, subPath, after, excludePathList, excludePathCount, extensionList, extensionCount);

    struct FilePathPair *current = list.next;
    while (current != NULL)
    {
        // arrangeBaseDir/from 경로 만들기
        char *fromPath = malloc(strlen(arrangeBaseDir) + 1 + strlen(current->from) + 1);
        strcpy(fromPath, arrangeBaseDir);
        strcat(fromPath, "/");
        strcat(fromPath, current->from);

        // realOutputPath/to 경로 만들기
        char *toPath = malloc(strlen(realOutputPath) + 1 + strlen(current->to) + 1);
        strcpy(toPath, realOutputPath);
        strcat(toPath, "/");
        strcat(toPath, current->to);

#if DEBUG
        printf("%s -> %s\n", fromPath, toPath);
#endif

        // fromPath 파일을 toPath 파일로 복사
        bool result;
        result = createDirForFile(toPath);
        if (!result)
        {
            printf("error: createDirForFile %d\n", errno);
        }

        result = copyFile(fromPath, toPath);
        if (!result)
        {
            printf("error: copyFile %d\n", errno);
        }

        free(fromPath);
        free(toPath);

        struct FilePathPair *next = current->next;
        free(current);
        current = next;
    }

    printf("%s arranged\n", dirPath);

    free(arrangeBaseDir);
    free(realOutputPath);
    return true;
}