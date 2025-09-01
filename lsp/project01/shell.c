#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

// input 에서 처음으로 만나는 value 를 \0 으로 바꾸고, 그 위치를 반환
char* strcut(char *input, char value) 
{
    char *space = strchr(input, value);
    if (space != NULL) 
    {
        *space = '\0';
        return space + 1;
    }
    return input + strlen(input);
}

// 쉘 프롬프트 출력
int shellPrompt(char *buffer, int size) 
{
    printf(SHELL_PROMPT);
    int i = 0;
    for (; i < size - 1; i++) 
    {
        buffer[i] = getchar();
        if (buffer[i] == EOF) 
        {
            return EOF;
        }
        if (buffer[i] == '\n') 
        {
            break;
        }
    }
    buffer[i] = '\0';
    return i;
}

// 명령어 문자열을 argc, argv 형태로 변환
// 공백 기준으로 arg 구분, 큰따옴표와 작은따옴표 사이의 공백 문자는 arg 의 일부로 취급
// 성공시 true, 올바르지 않은 명령어 형태라면 false 반환
bool lineToArgs(char* buffer, int *argc, char *argv[], int maxArgc) 
{
    *argc = 0;

    while (true) 
    {
        if (*argc >= maxArgc) 
        {
            return false;
        }

        if (*buffer == '\0') // 문자열 끝 종료
        {
            return true;
        }
        else if (*buffer == ' ') // 공백 무시
        { 
            buffer++;
        }
        else if (*buffer == '\"' || *buffer == '\'') // 따옴표 시작
        { 
            char quote = *buffer; // 따옴표 저장
            buffer++; // 따옴표 다음 문자
            argv[*argc] = buffer; // 시작 위치 argv 에 저장
            (*argc)++; // argc 증가

            while (true) 
            {
                if (*buffer == '\0') // 닫는 따옴표 없이 문자열 종료는 잘못된 형태
                { 
                    return false;
                }
                else if (*buffer == quote) // 닫는 따옴표 발견
                { 
                    *buffer = '\0'; // 닫는 따옴표를 arg 종료 위치로 지정
                    buffer++;
                    break;
                }
                else 
                {
                    buffer++;
                }
            }
        }
        else // 일반 문자열 시작
        { 
            argv[*argc] = buffer; // 시작 위치 argv 에 저장
            (*argc)++; // argc 증가

            while (true) 
            {
                if (*buffer == '\0') // 문자열 종료
                { 
                    return true;
                }
                else if (*buffer == ' ') // 공백 발견시 현재 arg 종료
                { 
                    *buffer = '\0'; // 띄어쓰기를 arg 종료 위치로 지정
                    buffer++;
                    break;
                }
                else 
                {
                    buffer++;
                }
            }
        }
    }

    return true;
}