#include "shell.h"
#include "tree.h"
#include "arrange.h"
#include "file.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

// tree 명령어의 help
void printTreeHelp() {
    printf(" > tree <DIR_PATH> [OPTION]...\n");
    printf("   <none> : Display the directory structure recursively if <DIR_PATH> is a directory\n");
    printf("   -s : Display the directory structure recursively if <DIR_PATH> is a directory, including the size of each file\n");
    printf("   -p : Display the directory structure recursively if <DIR_PATH> is a directory, including the permissions of each directory and file\n");
}

// arrange 명령어의 help
void printArrangeHelp() {
    printf(" > arrange <DIR_PATH> [OPTION]...\n");
    printf("   <none> : Arrange the directory if <DIR_PATH> is a directory\n");
    printf("   -d <output_path> : Specify the output directory <output_path> where <DIR_PATH> will be arranged if <DIR_PATH> is a directory\n");
    printf("   -t <seconds> : Only arrange files that were modified more than <seconds> seconds ago\n");
    printf("   -x <exclude_path1, exclude_path2, ...> : Arrange the directory if <DIR_PATH> is a directory except for the files inside <exclude_path> directory\n");
    printf("   -e <extension1, extension2, ...> : Arrange the directory with the specified extension <extension1, extension2,...>\n");
}

// 전체 help 메세지 출력
void printDefault() {
    printf("Usage:\n");
    printTreeHelp();
    printArrangeHelp();
}

// help 명령어 처리
bool help(char *input) {
    if (strcmp(input, "tree") == 0) {
        printf("Usage:\n");
        printTreeHelp();
        return true;
    }
    if (strcmp(input, "arrange") == 0) {
        printf("Usage:\n");
        printArrangeHelp();
        return true;
    }

    return false;
}

int main() {
    // 쉘 루프
    char input[6000];
    while (true) 
    {
        // 입력 받기
        int result = shellPrompt(input, 6000);
        if (result == EOF) // EOF 발견 시 프로그램 종료
        {
            return 0;
        }

        // 입력 문자열에서 처음으로 마주치는 공백 문자를 NULL 문자로 치환
        char* next = strcut(input, ' ');

        // 아무 입력도 없는 경우, 다시 입력 받기
        if (strcmp(input, "") == 0) 
        {
            continue;
        }
        else if (strcmp(input, "exit") == 0) // 종료 명령어
        {
            return 0;
        }
        else if (strcmp(input, "tree") == 0) // tree 명령어
        {
            if (!execTree(next))
                printf("Usage: tree <DIR_PATH> [OPTION]\n");
        }
        else if (strcmp(input, "arrange") == 0) // arrange 명령어
        {
            if (!execArrange(next))
                printf("Usage: arrange <DIR_PATH> [OPTION]\n");
        }
        else if (strcmp(input, "help") == 0) // help 명령어
        {
            if (!help(next))
                printDefault();
        }
        else // 알 수 없는 명령어
        {
            printDefault();
        }
    }
    return 0;
}