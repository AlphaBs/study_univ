#ifndef FILE_H

#define FILE_H

#include <sys/stat.h>
#include <stdbool.h>

// 경로 처리
bool isSubDir(const char *dir1, const char *dir2);
char *getHomeDir();
void getShellRealPath(char *result, const char *path);
void replaceHomeDir(char *result, const char *path);
void getDirName(char *result, const char *path);
bool getParentDir(char *result, const char *path);

// 파일 처리
void printPermission(mode_t mode);
bool createDirForFile(const char *filePath);
bool copyFile(const char *src, const char *dest);

#endif