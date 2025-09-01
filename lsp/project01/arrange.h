#ifndef ARRANGE_H
#define ARRANGE_H

#include <linux/limits.h>

bool execArrange(char* input);

// from 경로에서 to 경로로 복사하는 작업
// 링크드 리스트로 표현
struct FilePathPair
{
    char from[PATH_MAX];
    char to[PATH_MAX];

    struct FilePathPair *prev;
    struct FilePathPair *next;
};

#endif