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
#include <linux/limits.h>

// 절대경로 dir1 와 dir2 를 비교하여, 같은 경로이거나 dir1 하위에 dir2 가 있는지 확인 
bool isSubDir(const char *dir1, const char *dir2) {
    if (dir1 == NULL || dir2 == NULL || dir1[0] != '/' || dir2[0] != '/') {
        return false;
    }

    size_t dir1Length = strlen(dir1);
    size_t dir2Length = strlen(dir2);

    // trailing slash 제거
    while (dir1Length > 0 && dir1[dir1Length - 1] == '/') {
        dir1Length--;
    }
    while (dir2Length > 0 && dir2[dir2Length - 1] == '/') {
        dir2Length--;
    }

    // dir2 경로 길이가 dir1 보다 짧은 경우, dir1 하위에 dir2 가 올 수 없음
    if (dir2Length < dir1Length) {
        return false;
    }

    // dir2 앞부분이 dir1 으로 시작하지 않는다면, dir1 하위에 dir2 가 올 수 없음
    if (strncmp(dir1, dir2, dir1Length) != 0) {
        return false;
    }

    // dir2 가 dir1 으로 시작하면서 길이가 같다면, 완전히 같은 문자열
    if (dir1Length == dir2Length) {
        return true;
    }

    // dir2 가 dir1 으로 시작하고 직후에 slash 가 온다면, dir1 하위에 dir2 가 있다는 의미
    return dir2[dir1Length] == '/';
}

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

// 경로에서 디렉토리 이름을 추출
void getDirName(char *result, const char *path) {
    int pathLength = (int) strlen(path);
    if (pathLength == 0) {
        strcpy(result, "");
        return;
    }

    // 끝나는 '/' 건너뛰기
    int end = pathLength - 1;
    while (end >= 0 && path[end] == '/') {
        end--;
    }

    // '/' 으로만 이루어진 문자열
    if (end < 0) {
        strcpy(result, "");
        return;
    }

    // 마지막 '/' 찾기
    int start = end;
    while (start >= 0 && path[start] != '/') {
        start--;
    }

    if (start < 0) {
        // 마지막 '/' 가 없는 경우
        start = 0;
    } else {
        // '/' 다음으로 이동
        start++;
    }

    // [start, end] 문자열 복사
    int length = end - start + 1;
    strncpy(result, path + start, length);
    result[length] = '\0';
}

// 주어진 파일 경로에서 부모 디렉토리 경로 추출
bool getParentDir(char *result, const char *path) {
    int pathLength = (int) strlen(path);
    int end = pathLength - 1;
    while (end >= 0 && path[end] != '/') end--;

    if (end < 0) {
        strcpy(result, "");
        return false;
    }

    while (end >= 0 && path[end] == '/') end--;
    if (end < 0) {
        strcpy(result, "/");
        return false;
    }

    int length = end + 1;
    strncpy(result, path, length);
    result[length] = '\0';
    return true;
}

// filePath 의 부모 디렉토리가 존재하지 않는다면 생성
bool createDirForFile(const char *filePath) {
    // 부모 디렉토리 경로 찾기
    char dirPath[PATH_MAX];
    getParentDir(dirPath, filePath);
    if (dirPath[0] == '\0') {
        // 부모 디렉토리가 없는 경우, 즉 cwd 바로 아래에 파일이 있는 경우, 작업 필요 없음
        return true;
    }

    // 존재하지 않는다면 생성
    errno = 0;
    int result = mkdir(dirPath, 0777);
    return result != -1 || errno == EEXIST;
}

// 파일 src 를 dst 로 복사
bool copyFile(const char *src, const char *dest) {
    // src 열기
    int srcFile = open(src, O_RDONLY);
    if (srcFile < 0) {
        return false;
    }

    // dest 열기
    int destFile = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (destFile < 0) {
        close(srcFile);
        return false;
    }

    // 파일 복사
    char buffer[4096];
    ssize_t readBytes;
    while ((readBytes = read(srcFile, buffer, sizeof(buffer))) > 0) {
        ssize_t written = write(destFile, buffer, readBytes);
        if (written < 0) {
            close(srcFile);
            close(destFile);
            return false;
        }
    }

    close(srcFile);
    close(destFile);
    return true;
}
