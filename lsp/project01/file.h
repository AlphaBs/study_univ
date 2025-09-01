#ifndef FILE_H

#define FILE_H

#include <sys/stat.h>
#include <stdbool.h>

// 경로 처리
char* getHomeDir();
char* getShellRealPath(const char* path);
char* replaceHomeDir(const char* path);
char* getDirName(const char* path);

// 파일 처리
void printPermission(mode_t mode);
bool createDirForFile(const char *filePath);
bool copyFile(const char *src, const char *dest);

#endif