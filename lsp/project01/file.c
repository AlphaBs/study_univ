#define _GNU_SOURCE

#include "file.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pwd.h>

// $HOME 환경변수 반환, free 필요 없음, NULL 반환하는 경우 없음
char *getHomeDir() 
{
    char *path;
    if ((path = getenv("HOME")) == NULL) 
    {
        path = getpwuid(getuid())->pw_dir;
    }
    return path;
}

// ~ 를 $HOME 으로 바꾸어줌, free 필요
char* replaceHomeDir(const char* path)
{
    char *homeDir = getHomeDir();
    int homeDirLength = strlen(homeDir);
    
    if (path[0] == '~')
    {
        if (path[1] == '\0') // ~
        {
            char *realPath = malloc(sizeof(char) * (homeDirLength + 1));
            strcpy(realPath, homeDir);
            return realPath;
        }
        else if (path[1] == '/') // ~/, ~//, ~/a
        {
            while (*path != '/') path++; // ~ 다음 문자로 이동
            int remainPathLength = strlen(path);

            // <homeDir>/<path> 만들기
            char* fullPath = malloc(sizeof(char) * (homeDirLength + 1 + remainPathLength + 1));
            strcpy(fullPath, homeDir);
            fullPath[homeDirLength] = '/';
            strcpy(fullPath + homeDirLength + 1, path);

            return fullPath;
        }
    }

    char* result = malloc(sizeof(char) * strlen(path) + 1);
    strcpy(result, path);
    return result;
}

// . .. ~ 문자를 절대경로로 바꾸어줌, free 필요, 실패시 NULL 반환
char* getShellRealPath(const char* path) 
{
    char* p1 = replaceHomeDir(path);
    char* p2 = realpath(p1, NULL);
    free(p1);
    return p2;
}

// 경로에서 디렉토리 이름을 추출, free 필요, NULL 반환하는 경우 없음
// 'a/d' -> 'd'
// '/a/d/' -> 'd'
// 'a/d/////' -> 'd
// 'a//////d' -> 'd'
// '/' -> ''
// '////' -> ''
// '' -> ''
// 'd' -> 'd'
// 'd//' -> 'd'
// '/d' -> 'd'
// '/d/' -> 'd'
char* getDirName(const char* path)
{
    int pathLength = strlen(path);
    if (pathLength == 0)
    {
        return strdup("");
    }

    // 끝나는 '/' 건너뛰기
    int end = pathLength - 1;
    while (end >= 0 && path[end] == '/') 
    {
        end--;
    }

    // '/' 으로만 이루어진 문자열
    if (end < 0)
    {
        return strdup("");
    }

    // 마지막 '/' 찾기
    int start = end;
    while (start >= 0 && path[start] != '/') 
    {
        start--;
    }

    if (start < 0) 
    {
        // 마지막 '/' 가 없는 경우
        start = 0;
    }
    else 
    {
        // '/' 다음으로 이동
        start++;
    }

    // [start, end] 문자열 복사
    int length = end - start + 1;
    char* dirName = malloc(length + 1);
    strncpy(dirName, path + start, length);
    dirName[length] = '\0';
    return dirName;
}

// 경로에서 부모 디렉토리 경로만 추출, free 필요, NULL 반환 가능
char* getParentDir(const char* path)
{
    int pathLength = strlen(path);
    int end = pathLength - 1;
    while (end >= 0 && path[end] != '/') end--;

    if (end < 0)
    {
        return NULL;
    }
    
    int length = end + 1;
    char* parentDir = malloc(length + 1);
    strncpy(parentDir, path, length);
    parentDir[length] = '\0';
    return parentDir;
}

// filePath 의 부모 디렉토리가 존재하지 않는다면 생성
bool createDirForFile(const char *filePath)
{
    // 부모 디렉토리 경로 찾기
    char *dirPath = getParentDir(filePath);
    if (dirPath == NULL) 
    {
        // 부모 디렉토리가 없는 경우, 즉 cwd 바로 아래에 파일이 있는 경우, 작업 필요 없음
        return true;
    }
    
    // 존재하지 않는다면 생성
    errno = 0;
    int result = mkdir(dirPath, 0777);
    free(dirPath);
    return result != -1 || errno == EEXIST;
}

// 파일 모드를 출력, 예시: drwxrwxrwx
void printPermission(mode_t mode) 
{
    if (S_ISDIR(mode))
        putchar('d');
    else
        putchar('-');

    // user
    if (mode & S_IRUSR)
        putchar('r');
    else
        putchar('-');
    if (mode & S_IWUSR)
        putchar('w');
    else
        putchar('-');
    if (mode & S_IXUSR)
        putchar('x');
    else
        putchar('-');

    // group
    if (mode & S_IRGRP)
        putchar('r');
    else
        putchar('-');
    if (mode & S_IWGRP)
        putchar('w');
    else
        putchar('-');
    if (mode & S_IXGRP)
        putchar('x');
    else
        putchar('-');

    // other
    if (mode & S_IROTH)
        putchar('r');
    else
        putchar('-');
    if (mode & S_IWOTH)
        putchar('w');
    else
        putchar('-');
    if (mode & S_IXOTH)
        putchar('x');
    else
        putchar('-');
}

// 파일 src 를 dst 로 복사
bool copyFile(const char *src, const char *dest)
{
    // src 열기
    int srcFile = open(src, O_RDONLY);
    if (srcFile == -1)
    {
        return false;
    }

    // dest 열기
    int destFile = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (destFile == -1)
    {
        close(srcFile);
        return false;
    }

    // 파일 복사
    char buffer[4096];
    ssize_t readBytes;
    while ((readBytes = read(srcFile, buffer, sizeof(buffer))) > 0)
    {
        ssize_t written = write(destFile, buffer, readBytes);
        if (written == -1)
        {
            close(srcFile);
            close(destFile);
            return false;
        }
    }

    close(srcFile);
    close(destFile);
    return true;
}