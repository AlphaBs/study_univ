#define _GNU_SOURCE

#include "ext2.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>

static FILE *fp;
static struct Ext2Superblock sb;
static bool optRecursive;
static bool optSize;
static bool optPermission;

static int totalFileCount;
static int totalDirectoryCount;

static void printPermission(uint16_t mode)
{
    if ((mode & EXT2_INODE_TYPE_MASK) == EXT2_INODE_TYPE_DIR)
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

// 노드 정보 출력
static void printNodeStat(struct Ext2INode *node)
{
    if (optPermission && optSize)
    {
        putchar('[');
        printPermission(node->typeAndPermission);
        printf(" %u] ", node->sizeLowerBits);
    }
    else if (optPermission)
    {
        putchar('[');
        printPermission(node->typeAndPermission);
        putchar(']');
        putchar(' ');
    }
    else if (optSize)
    {
        printf("[%u] ", node->sizeLowerBits);
    }
}

// 트리 재귀 출력
static bool printTree(struct Ext2INode *node, int depth, bool *tree)
{
    bool error = false;

    // 디렉토리의 내용을 링크드 리스트 형태로 받아옴
    struct Ext2DirectoryEntryList* list = ext2ScanDir(fp, &sb, node);
    if (list == NULL)
    {
        return false;
    }

    // 링크드 리스트 순회
    struct Ext2DirectoryEntryList* currentNode = list->next;
    while (currentNode != NULL)
    {
        // 트리를 계속 출력할지? 출력하지 않으면 continue 가 아니라 끝에서 continue 하고 다음 node 로 넘어가는 작업 필요함
        // . .. lost+found 는 출력하지 않음
        // 오류 발생시 더 이상 트리를 출력하지 않음
        if (!error && 
            strcmp(currentNode->entry.name, ".") != 0 && 
            strcmp(currentNode->entry.name, "..") != 0 && 
            strcmp(currentNode->entry.name, "lost+found") != 0) 
        {
            // 트리 선 그리기
            for (int d = 0; d < depth; d++)
            {
                if (tree[d])
                {
                    printf("│   ");
                }
                else
                {
                    printf("    ");
                }
            }

            if (currentNode->next == NULL) // 마지막 노드면 ㄴ 선
            {
                printf("└─ ");
                tree[depth] = false;
            }
            else // 마지막 노드가 아니라면 ㅏ 선
            {
                printf("├─ ");
                tree[depth] = true;
            }

            // inode 읽기
            struct Ext2INode inode;
            if (ext2ReadNode(fp, &sb, currentNode->entry.inode, &inode) == 0)
            {
                if ((inode.typeAndPermission & EXT2_INODE_TYPE_MASK) == EXT2_INODE_TYPE_DIR)
                {
                    totalDirectoryCount++;

                    // 파일 크기, 접근 권한 출력
                    printNodeStat(&inode);
                    printf("%s\n", currentNode->entry.name);

                    // 재귀호출 옵션
                    if (optRecursive)
                    {
                        // 재귀호출이 오류를 반환하면 더 이상 트리를 출력하지 않음
                        if (!printTree(&inode, depth + 1, tree))
                        {
                            error = true;
                        }
                    }
                }
                else
                {
                    totalFileCount++;

                    // 파일 크기, 접근 권한 출력
                    printNodeStat(&inode);
                    printf("%s\n", currentNode->entry.name);
                }
            }
            else
            {
                // 오류, 트리 출력 중단
                fprintf(stderr, "failed to inode %u\n", currentNode->entry.inode);
                error = true;
            }
        }

        // 다음 노드로 이동
        struct Ext2DirectoryEntryList* nextNode = currentNode->next;
        free(currentNode);
        currentNode = nextNode;
    }

    free(list);
    return !error;
}

static void printTreeUsage()
{
    printf("Usage: tree <PATH> [OPTION]...\n");
}

// argv: PATH, OPTION1, OPTION2 형태
void execTree(FILE *imgFile, int argc, char **argv)
{
    // 잘못된 사용법 바로 리턴
    if (argc == 0)
    {
        printTreeUsage();
        return;
    }

    // 옵션 파싱
    fp = imgFile;
    optRecursive = false;
    optSize = false;
    optPermission = false;
    char *path = argv[0];
    int opt;
    while ((opt = getopt(argc, argv, "rsp")) != -1)
    {
        switch (opt)
        {
        case 'r':
            optRecursive = true;
            break;
        case 's':
            optSize = true;
            break;
        case 'p':
            optPermission = true;
            break;
        default: // 올바르지 않은 옵션이 들어왔을 경우 Usage 출력 후 프롬프트 재출력
            printTreeUsage();
            return;
        }
    }

    if (optind != argc) // 인자가 아직 남아있는 경우
    {
        printTreeUsage();
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
    int nodeAddress = ext2FindPath(fp, &sb, ROOT_DIRECTORY_INODE, path);
    if (nodeAddress == 0)
    {
        // fprintf(stderr, "can't find a path %s\n", path);
        printTreeUsage();
        return;
    }

    // 명세서: 디렉토리가 아닌 경우 에러메세지 출력 후 프롬프트 재출력
    struct Ext2INode node;
    if (ext2ReadNode(fp, &sb, nodeAddress, &node) < 0)
    {
        fprintf(stderr, "Error: failed to read inode '%u'\n", nodeAddress);
        return;
    }

    if ((node.typeAndPermission & EXT2_INODE_TYPE_MASK) != EXT2_INODE_TYPE_DIR)
    {
        fprintf(stderr, "Error: '%s' is not directory\n", path);
        return;
    }

    // 초기화
    bool tree[PATH_MAX];
    for (int i = 0; i < PATH_MAX; i++)
    {
        tree[i] = false;
    }
    totalDirectoryCount = 1;
    totalFileCount = 0;

    // 트리 출력
    printNodeStat(&node);
    printf("%s\n", path);
    bool result = printTree(&node, 0, tree);

    if (result)
    {
        printf("\n%d directories, %d files\n\n", totalDirectoryCount, totalFileCount);
    }
}