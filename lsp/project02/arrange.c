#define _GNU_SOURCE

#include "arrange.h"
#include "shell.h"
#include "debug.h"
#include "file.h"
#include "daemonlog.h"
#include "daemonconfig.h"

#include <assert.h>
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

#define SELECT_NONE 0
#define SELECT_1 1
#define SELECT_2 2
#define SELECT_BOTH 3

static DaemonConfig *daemonConfig;

int selectPair(int mode, struct FilePathPair *p1, struct FilePathPair *p2)
{
    p1->selected = (mode == SELECT_1 || mode == SELECT_BOTH);
    p2->selected = (mode == SELECT_2 || mode == SELECT_BOTH);
    return mode;
}

// 충돌 해결 방법 선택
int resolveConflict(struct FilePathPair *p1, struct FilePathPair *p2)
{
    // 중복되는 파일 무시
    if (daemonConfig->duplicatedFileMode == DUPLICATED_FILE_MODE_IGNORE)
    {
        return selectPair(SELECT_NONE, p1, p2);
    }

    // 수정 시간 동일
    if (p1->mtime == p2->mtime)
    {
        return selectPair(SELECT_BOTH, p1, p2);
    }
    // p1 이 최신파일
    else if (p1->mtime > p2->mtime)
    {
        if (daemonConfig->duplicatedFileMode == DUPLICATED_FILE_MODE_LATEST)
        {
            return selectPair(SELECT_1, p1, p2);
        }
        else if (daemonConfig->duplicatedFileMode == DUPLICATED_FILE_MODE_OLDEST)
        {
            return selectPair(SELECT_2, p1, p2);
        }
    }
    // p2 이 최신파일
    else
    {
        if (daemonConfig->duplicatedFileMode == DUPLICATED_FILE_MODE_LATEST)
        {
            return selectPair(SELECT_2, p1, p2);
        }
        if (daemonConfig->duplicatedFileMode == DUPLICATED_FILE_MODE_OLDEST)
        {
            return selectPair(SELECT_1, p1, p2);
        }
    }

    return selectPair(SELECT_NONE, p1, p2);
}

// 개별 파일 처리
void arrangeFile(struct FilePathPair *list, const char *subPath)
{
    assert(list != NULL);

    // 파일 이름에서 마지막 점의 위치 찾기
    int subPathLength = (int)strlen(subPath);
    int lastDotIndex = -1;
    for (int j = subPathLength - 1; j >= 0; j--)
    {
        if (subPath[j] == '.')
        {
            lastDotIndex = j;
            break;
        }
    }

    // 파일 이름에서 마지막 / 위치 찾기
    int lastSlashIndex = -1;
    for (int j = subPathLength - 1; j >= 0; j--)
    {
        if (subPath[j] == '/')
        {
            lastSlashIndex = j;
            break;
        }
    }

    // 확장자 없는 파일 무시
    int extLength = subPathLength - lastDotIndex - 1;
    if (extLength <= 0)
    {
        return;
    }

    // 확장자 목록에 포함되어 있는지 확인
    bool isIncluded = false;
    for (int j = 0; j < daemonConfig->extensionCount; j++)
    {
        if (strcmp(daemonConfig->extensionList[j], subPath + lastDotIndex + 1) == 0)
        {
            isIncluded = true;
            break;
        }
    }

    // 확장자 목록이 존재하면서 목록에 포함되지 않은 파일 무시
    if (!isIncluded && daemonConfig->extensionCount > 0)
    {
        return;
    }

    // config 파일, 로그 파일 제외
    if (strcmp(subPath, DAEMON_CONFIG_FILE_NAME) == 0)
    {
        return;
    }
    if (strcmp(subPath, DAEMON_LOG_FILE_NAME) == 0)
    {
        return;
    }

    // 파일 절대 경로 생성
    char fullPath[PATH_MAX];
    snprintf(fullPath, PATH_MAX, "%s/%s", daemonConfig->dirPath, subPath);

    // 파일 정보 가져오기
    struct stat fileStat;
    int statResult = stat(fullPath, &fileStat);
    if (statResult == -1)
    {
        printf("error: %d\n", errno);
        return;
    }

    // { subPath, 확장자/파일이름.확장자 } 생성
    struct FilePathPair *currentPair = malloc(sizeof(struct FilePathPair));
    currentPair->selected = true;
    currentPair->next = NULL;
    currentPair->mtime = fileStat.st_mtime;
    strcpy(currentPair->from, subPath);
    memset(currentPair->to, 0, sizeof(currentPair->to));

    int fileNameLength = lastDotIndex - lastSlashIndex - 1;
    if (fileNameLength <= 0)
    {
        // . 없는 파일 혹은 . 으로 시작하는 숨김 파일은 무시
        // strcat(currentPair->to, "_/");
        // strcat(currentPair->to, subPath + lastSlashIndex + 1);
        return;
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
    struct FilePathPair *conflictPair = list;
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
        // 둘 다 버리는 경우, 현재 FilePair 삭제
        free(currentPair);
        break;
    case SELECT_1:
        // 기존 파일 남기는 경우, 현재 FilePair 삭제
        free(currentPair);
        break;
    case SELECT_2:
        // 현재 파일 남기는 경우, conflictPair 를 현재 파일로 교체
        struct FilePathPair *next = conflictPair->next;
        memcpy(conflictPair, currentPair, sizeof(struct FilePathPair));
        conflictPair->next = next;
        free(currentPair);
        break;
    default:
        // 둘 다 유지하는 경우, 리스트 끝에 아이템 추가
        // 이때 conflictPair 는 리스트의 마지막 아이템
        conflictPair->next = currentPair;
        currentPair->next = NULL;
        break;
    }
}

// 디렉토리 정리
void arrangeDir(struct FilePathPair *list, const char *subPath)
{
    // dirPath/subPath 경로 생성
    char concatPath[PATH_MAX];
    snprintf(concatPath, PATH_MAX + 2, "%s/%s", daemonConfig->dirPath, subPath);

    // 절대 경로 생성
    char fullPath[PATH_MAX];
    realpath(concatPath, fullPath);

    // -x 옵션의 제외 경로 무시
    for (int i = 0; i < daemonConfig->excludePathCount; i++)
    {
        if (isSubDir(daemonConfig->excludePathList[i], fullPath))
        {
            return;
        }
    }

    struct dirent **namelist;
    int n = scandir(fullPath, &namelist, NULL, alphasort);
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
            char nextPath[PATH_MAX];
            if (strcmp(subPath, "") == 0)
            {
                strcpy(nextPath, namelist[i]->d_name);
            }
            else
            {
                snprintf(nextPath, PATH_MAX, "%s/%s", subPath, namelist[i]->d_name);
            }

            if (namelist[i]->d_type == DT_DIR)
            {
                arrangeDir(list, nextPath);
            }
            else
            {
                arrangeFile(list, nextPath);
            }
        }
        free(namelist[i]);
    }
    free(namelist);
}

// 정리 결과를 실제로 복사하지 않고 결과 리스트만 반환 (디버깅용)
void arrangeDry(DaemonConfig *config, struct FilePathPair *list)
{
    daemonConfig = config;
    const char subPath[1] = {'\0'};
    arrangeDir(list, subPath);
}

// 명령어 실행
void arrangeDaemon(DaemonConfig *config)
{
    // arranged 디렉토리 생성
    mkdir(config->outputPath, 0777);

    // 설정
    daemonConfig = config;

    // 정리 결과가 저장되는 리스트
    struct FilePathPair list;
    list.next = NULL;

    // 디렉토리 정리
    const char subPath[1] = {'\0'};
    arrangeDir(&list, subPath);

    // 로그 파일 열고 내용 읽기
    FILE *file = openDaemonLogFile(config, "r");
    DaemonLogList *logList;
    if (file != NULL)
    {
        logList = readDaemonLogFile(file);
        fclose(file);
    }
    else
    {
        logList = malloc(sizeof(DaemonLogList));
        logList->item.length = 0;
        logList->next = NULL;
    }

    // 리스트 순회
    struct FilePathPair *current = list.next;
    while (current != NULL)
    {
        // 선택된 파일만 복사, DUPLICATED_FILE_MODE_IGNORE 로 무시된 파일도 리스트에 존재함
        if (current->selected)
        {
            // <DIR_PATH>/from 경로 만들기
            char fromPath[PATH_MAX];
            snprintf(fromPath, sizeof(fromPath), "%s/%s", daemonConfig->dirPath, current->from);

            // <OUTPUT_PATH>/to 경로 만들기
            char toPath[PATH_MAX];
            snprintf(toPath, sizeof(toPath), "%s/%s", daemonConfig->outputPath, current->to);

            // 목적지 경로에 디렉토리 만들기
            bool result = createDirForFile(toPath);
            if (!result)
            {
                printf("error: createDirForFile %d\n", errno);
            }

            // 만약 toPath 에 이미 파일이 존재한다면, 충돌 해결
            bool shouldCopy = true;
            if (access(toPath, F_OK) == 0)
            {
                struct stat st;
                if (stat(toPath, &st) == 0)
                {
                    struct FilePathPair existingPair;
                    existingPair.mtime = st.st_mtime;
                    strcpy(existingPair.to, toPath);

                    int resolveResult = resolveConflict(&existingPair, current);

                    // 기존에 존재하던 파일을 선택, 복사를 건너뜀
                    if (resolveResult == SELECT_1)
                    {
                        shouldCopy = false;
                    }

                    // 둘 다 선택하지 않는 경우, 기존 파일을 남기는 것이 명세
                    // 현재 -m 옵션이 1 또는 2일 때 중복 파일이 정리되어 _arranged 디렉토리에 저장되었습니다.
                    // 그런데 이후 -m 값을 3으로 변경하면, 1) 이미 _arranged 폴더에 정리된 중복 파일들도 삭제해야 하는지, 2) 아니면 기존에 정리된 파일은 그대로 두고, 이후 새로 중복되는 파일만 정리하지 않으면 되는지.
                    // 위 두 가지 중 어떤 방식으로 처리해야 하는지가 궁금합니다.
                    // ==> 2번이 맞음. 기존에 정리된 파일은 그대로 두면 됨.
                    if (resolveResult == SELECT_NONE)
                    {
                        shouldCopy = false;
                    }
                }
            }

            // fromPath 파일을 toPath 파일로 복사
            if (shouldCopy)
            {
                result = copyFile(fromPath, toPath);
                if (!result)
                {
                    printf("error: copyFile %d\n", errno);
                }

                // 로그 작성
                appendDaemonLog(logList, config, time(NULL), fromPath, toPath);
            }
        }

        struct FilePathPair *next = current->next;
        free(current);
        current = next;
    }

    // 로그 파일 TRUNCATE 으로 다시 열고 저장
    file = openDaemonLogFile(config, "w");
    writeDaemonLogFile(file, logList);
    fclose(file);
    freeDaemonLogList(logList);
}
