#define _GNU_SOURCE

#include "shell.h"
#include "ext2.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static FILE *fp;
static struct Ext2Superblock sb;
static bool useOptN;
static int optN;

static void printContent(const struct Ext2INode* node)
{
    uint64_t blockSize = ext2GetBlockSize(&sb);
    char* buf = malloc(blockSize);
    if (buf == NULL)
    {
        fprintf(stderr, "printContent: malloc error\n");
        return;
    }

    int64_t read;
    int lineEnds = 0;
    bool done = false;
    for (uint32_t blockOffset = 0;; blockOffset++) 
    {
        // block 단위로 읽기
        read = ext2ReadContent(fp, &sb, node, blockOffset, buf);
        if (read <= 0) // 오류 혹은 파일 끝
        {
            break;
        }

        for (int i = 0; i < read; i++) // 한글자씩 출력
        {
            putchar(buf[i]);
            if (useOptN && buf[i] == '\n')
            {
                lineEnds++;
                if (lineEnds >= optN)
                {
                    done = true;
                    break;
                }
            }
        }

        if (done)
            break;
    }

    free(buf);
}

static void printPrintUsage()
{
    printf("Usage: print <PATH> [OPTION]...\n");
}

// argv: PATH, OPTION1, OPTION2 형태
void execPrint(FILE* imgFile, int argc, char **argv) 
{
    // 잘못된 사용법 바로 리턴
    if (argc == 0)
    {
        printPrintUsage();
        return;
    }

    // 옵션 파싱
    fp = imgFile;
    useOptN = false;
    optN = -1;
    char *path = argv[0];
    int opt;
    while ((opt = getopt(argc, argv, "n:")) != -1)
    {
        switch (opt)
        {
        case 'n':
            useOptN = true;
            optN = parseExactInt(optarg, -1);
            break;
        default: // 올바르지 않은 옵션이 들어왔을 경우 Usage 출력 후 프롬프트 재출력
            printPrintUsage();
            return;
        }
    }

    if (optind != argc) // 인자가 아직 남아있는 경우
    {
        printPrintUsage();
        return;
    }

    if (useOptN && optN < 0) 
    {
        printf("print: -n %d must be positive number\n", optN);
        return;
    }

    // sb 읽기
    if (ext2ReadSuperblock(fp, &sb) < 0)
    {
        fprintf(stderr, "Error: failed to read superblock\n");
        return;
    }

    // 명세서: 상대경로를 입력받음
    if (path[0] == '/')
    {
        fprintf(stderr, "Error: <PATH> should be relative path, %s\n", path);
        return;
    }

    // 명세서: 올바르지 않은 경로, 존재하지 않는 경로는 Usage 출력 후 프롬프트 재출력
    // 경로 찾기는 디렉토리 내용을 링크드 리스트로 읽어서 경로 세그먼트를 하나씩 따라가면서 최종 inode address 를 찾아옴
    int nodeAddress = ext2FindPath(fp, &sb, ROOT_DIRECTORY_INODE, path);
    if (nodeAddress == 0)
    {
        // fprintf(stderr, "can't find a path %s\n", path);
        printPrintUsage();
        return;
    }

    // 명세서: 디렉토리가 아닌 경우 에러메세지 출력 후 프롬프트 재출력
    struct Ext2INode node;
    if (ext2ReadNode(fp, &sb, nodeAddress, &node) < 0)
    {
        fprintf(stderr, "Error: failed to read inode '%u'\n", nodeAddress);
        return;
    }

    if ((node.typeAndPermission & EXT2_INODE_TYPE_MASK) != EXT2_INODE_TYPE_REG)
    {
        fprintf(stderr, "Error: '%s' is not file\n", path);
        return;
    }

    // n = 0 이라면 출력이 필요하지 않음
    if (useOptN && optN == 0)
    {
        return;
    }

    // 내용 출력
    printContent(&node);
}