#include "ext2.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>

void copySuperblockFields(const char *buf, struct Ext2Superblock *sb)
{
    memcpy(&sb->inodeCount,     buf +  0, 4);
    memcpy(&sb->blockCount,     buf +  4, 4);
    memcpy(&sb->blockSizeInLog, buf + 24, 4);
    memcpy(&sb->blocksPerGroup, buf + 32, 4);
    memcpy(&sb->inodesPerGroup, buf + 40, 4);
    memcpy(&sb->magic,          buf + 56, 2);
    memcpy(&sb->majorVersion,   buf + 76, 4);
    memcpy(&sb->inodeSize,      buf + 88, 2);
}

int ext2ReadSuperblock(FILE *fp, struct Ext2Superblock *sb)
{
    // superblock 으로 이동
    char buf[SUPERBLOCK_SIZE];
    if (fseek(fp, SUPERBLOCK_OFFSET, SEEK_SET) != 0)
    {
        perror("fseek to superblock");
        return -1;
    }

    // superblock 읽기
    if (fread(buf, SUPERBLOCK_SIZE, 1, fp) != 1)
    {
        perror("fread to superblock");
        return -1;
    }

    // 필드 복사
    copySuperblockFields(buf, sb);

    // magic 확인
    if (sb->magic != SUPERBLOCK_MAGIC)
    {
        fprintf(stderr, "mismatch magic: %x\n", sb->magic);
        return -1;
    }

    return 0;
}

void ext2PrintSuperblock(const struct Ext2Superblock *sb)
{
    printf("inodeCount: %u\n", sb->inodeCount);
    printf("blockCount: %u\n", sb->blockCount);
    printf("blockSizeInLog: %u\n", sb->blockSizeInLog);
    printf("blocksPerGroup: %u\n", sb->blocksPerGroup);
    printf("inodesPerGroup: %u\n", sb->inodesPerGroup);
    printf("magic: %hu\n", sb->magic);
    printf("majorVersion: %u\n", sb->majorVersion);
    printf("inodeSize: %hu\n", sb->inodeSize);
}

uint64_t ext2GetBlockSize(const struct Ext2Superblock *sb)
{
    uint32_t blockSize = 1024 << sb->blockSizeInLog;
    return blockSize;
}

int ext2ReadNode(
    FILE *fp,
    const struct Ext2Superblock *sb,
    uint32_t nodeAddress,
    struct Ext2INode *node)
{
    uint32_t blockSize = ext2GetBlockSize(sb);

    // inode 가 속한 block group number 찾기
    uint32_t nodeBlockGroupNum = (nodeAddress - 1) / sb->inodesPerGroup;

    // block group descriptor table 찾기
    uint32_t gdtBlockNum = (SUPERBLOCK_OFFSET / blockSize) + 1;
    off_t gdtOffset = (off_t)gdtBlockNum * blockSize;

    // group descriptor 찾기
    off_t blockDescriptorOffset = gdtOffset + (nodeBlockGroupNum * GROUPDESC_SIZE);

    // group descriptor 이동하고 읽기
    char descBuf[GROUPDESC_SIZE];
    if (fseek(fp, blockDescriptorOffset, SEEK_SET) != 0)
    {
        perror("fseek to blockDescriptorOffset error");
        return -1;
    }

    if (fread(descBuf, GROUPDESC_SIZE, 1, fp) != 1)
    {
        perror("fread groupDesc error");
        return -1;
    }

    // group descriptor 필드 복사
    struct Ext2GroupDesc groupDesc;
    memcpy(&groupDesc.blockBitmapBlock, descBuf + 0, 4);
    memcpy(&groupDesc.inodeBitmapBlock, descBuf + 4, 4);
    memcpy(&groupDesc.inodeTableBlock,  descBuf + 8, 4);

    // inode offset 찾기
    off_t inodeTableOffset = groupDesc.inodeTableBlock * blockSize;
    uint32_t inodeIndexInGroup = (nodeAddress - 1) % sb->inodesPerGroup;
    off_t inodeOffset = inodeTableOffset + (inodeIndexInGroup * sb->inodeSize);

    // inode 이동
    if (fseek(fp, inodeOffset, SEEK_SET) != 0)
    {
        perror("fseek to inodeOffset error");
        return -1;
    }

    // inode 버퍼 생성
    char *nodeBuf = malloc(sb->inodeSize);
    if (nodeBuf == NULL)
    {
        perror("malloc node error");
        return -1;
    }

    // inode 읽기
    if (fread(nodeBuf, sb->inodeSize, 1, fp) != 1)
    {
        free(nodeBuf);
        perror("fread node error");
        return -1;
    }

    // inode 필드 복사
    memcpy(&node->typeAndPermission,          nodeBuf +  0, 2);
    memcpy(&node->sizeLowerBits,              nodeBuf +  4, 4);
    memcpy(&node->directBlockPointer,         nodeBuf + 40, 4 * 12);
    memcpy(&node->singleIndirectBlockPointer, nodeBuf + 88, 4);
    memcpy(&node->doubleIndirectBlockPointer, nodeBuf + 92, 4);
    memcpy(&node->tripleIndirectBlockPointer, nodeBuf + 96, 4);

    free(nodeBuf);
    return 0;
}

void ext2PrintNode(const struct Ext2INode *node)
{
    printf("typeAndPermission: %x\n", node->typeAndPermission);
    printf("sizeLowerBits: %x\n", node->sizeLowerBits);
    for (int i = 0; i < 12; i++)
        printf("directBlockPointer[%d]: %x\n", i, node->directBlockPointer[i]);
    printf("singleIndirect: %x\n", node->singleIndirectBlockPointer);
    printf("doubleIndirect: %x\n", node->doubleIndirectBlockPointer);
    printf("tripleIndirect: %x\n", node->tripleIndirectBlockPointer);
}

// blockAddress 를 bufSize 만큼 읽어서 buf 에 저장
int ext2ReadBlock(
    FILE *fp,
    const struct Ext2Superblock *sb,
    uint32_t blockAddress,
    char *buf,
    uint32_t bufSize)
{
    uint32_t blockSize = ext2GetBlockSize(sb);
    off_t blockOffset = blockAddress * blockSize;

    if (fseek(fp, blockOffset, SEEK_SET) != 0)
    {
        perror("fseek to blockOffset");
        return -1;
    }

    if (fread(buf, 1, bufSize, fp) != bufSize)
    {
        perror("fread block");
        return -1;
    }

    return 0;
}

uint32_t getIndirectPointer(const struct Ext2INode *node, int indirectLevel)
{
    if (indirectLevel == 1)
        return node->singleIndirectBlockPointer;
    else if (indirectLevel == 2)
        return node->doubleIndirectBlockPointer;
    else if (indirectLevel == 3)
        return node->tripleIndirectBlockPointer;
    else
        return 0;
}

uint64_t powInt(uint64_t base, int count)
{
    uint64_t result = 1;
    for (int i = 0; i < count; i++)
    {
        result *= base;
    }
    return result;
}

// indirectLevel 만큼 indirect 된 indirectPointer 로 부터,
// blockOffset 번째 block 의 실제 block address 를 찾음
int64_t findBlockAddressFromIndirectPointer(
    FILE *fp,
    const struct Ext2Superblock *sb,
    uint32_t indirectPointer,
    uint32_t blockPointerOffset,
    int indirectLevel)
{
    // 할당되지 않은 indirectPointer 무시
    if (indirectPointer == 0)
        return 0;

    // block 읽고 저장할 버퍼
    uint64_t blockSize = ext2GetBlockSize(sb);
    char *blockBuf = malloc(blockSize);

    if (blockBuf == NULL)
    {
        perror("malloc findIndirectBlockAddress");
        return -1;
    }

    // indirectPointer 가 가르키는 block 읽기
    if (ext2ReadBlock(fp, sb, indirectPointer, blockBuf, blockSize) != 0)
    {
        free(blockBuf);
        return -1;
    }

    // block 안 pointer 하나가 총 몇개의 direct pointer 를 가지고 있는지 확인
    uint64_t totalPointersInSubBlock = powInt(blockSize / BLOCK_POINTER_SIZE, indirectLevel - 1);
    // blockPointerOffset 이 현재 block 에서 몇번째 pointer 안에 속해있는지 확인
    uint32_t currentPointerIndexInBlock = blockPointerOffset / totalPointersInSubBlock;

    // 읽기
    uint32_t blockAddress;
    memcpy(&blockAddress, blockBuf + currentPointerIndexInBlock * BLOCK_POINTER_SIZE, 4);
    free(blockBuf);

    if (blockAddress == 0)
    {
        // 할당되지 않은 blockAddress 무시
        // 다음 indirectLevel 로 넘어가지 않도록 처리
        return 0;
    }
    else if (indirectLevel == 1)
    {
        // single indirect 이면 blockAddress 가 찾는 값
        return blockAddress;
    }
    else
    {
        // double, triple 이라면 blockAddress 로 다시 indirect 찾아가야함
        blockPointerOffset = blockPointerOffset % totalPointersInSubBlock;
        return findBlockAddressFromIndirectPointer(fp, sb, blockAddress, blockPointerOffset, indirectLevel - 1);
    }
}

// blockPointerOffset 이 가르키는 block address 값을 찾음
int64_t findBlockAddress(
    FILE *fp,
    const struct Ext2Superblock *sb,
    const struct Ext2INode *node,
    uint32_t blockPointerOffset)
{
    uint64_t blockSize = ext2GetBlockSize(sb);

    // directBlockPointer
    if (blockPointerOffset < DIRECT_BLOCK_POINTER_COUNT)
    {
        return node->directBlockPointer[blockPointerOffset];
    }
    uint32_t currentOffset = blockPointerOffset - DIRECT_BLOCK_POINTER_COUNT;

    // single indirect
    uint64_t singleIndirectCapacity = blockSize / BLOCK_POINTER_SIZE;
    if (currentOffset < singleIndirectCapacity)
    {
        return findBlockAddressFromIndirectPointer(fp, sb, getIndirectPointer(node, 1), currentOffset, 1);
    }
    currentOffset -= singleIndirectCapacity;

    // double indirect
    uint64_t doubleIndirectCapacity = powInt(blockSize / BLOCK_POINTER_SIZE, 2);
    if (currentOffset < doubleIndirectCapacity)
    {
        return findBlockAddressFromIndirectPointer(fp, sb, getIndirectPointer(node, 2), currentOffset, 2);
    }
    currentOffset -= doubleIndirectCapacity;

    // triple indirect
    uint64_t tripleIndirectCapacity = powInt(blockSize / BLOCK_POINTER_SIZE, 3);
    if (currentOffset < tripleIndirectCapacity)
    {
        return findBlockAddressFromIndirectPointer(fp, sb, getIndirectPointer(node, 3), currentOffset, 3);
    }

    // 최대 크기를 벗어남
    return -1;
}

// blockPointerOffset 이 가르키는 곳을 읽고, 읽은 바이트 수를 반환
// 끝을 넘어서면 0을 반환, 오류는 -1 반환
int64_t ext2ReadContent(
    FILE *fp,
    const struct Ext2Superblock *sb,
    const struct Ext2INode *node,
    uint32_t blockPointerOffset,
    char *buf)
{
    // block 이 파일의 끝을 벗어나는지 검사
    uint64_t blockSize = ext2GetBlockSize(sb);
    uint64_t currentBlockStartByteOffset = (uint64_t)blockPointerOffset * blockSize;
    if (currentBlockStartByteOffset >= node->sizeLowerBits)
    {
        return 0;
    }

    // 실제 block 의 위치를 찾음
    int64_t blockAddress = findBlockAddress(fp, sb, node, blockPointerOffset);
    if (blockAddress == 0) // hole
    {
        memset(buf, 0, blockSize);
        return blockSize;
    }
    else if (blockAddress < 0) // 오류
    {
        return -1;
    }

    // block 위치로부터 몇 바이트를 읽어야 하는지 결정
    uint64_t readSize = node->sizeLowerBits - currentBlockStartByteOffset;
    if (readSize > blockSize)
        readSize = blockSize;

    // block 읽기
    if (ext2ReadBlock(fp, sb, blockAddress, buf, readSize) != 0)
    {
        return -1;
    }
    return readSize;
}

// node 에서 byteOffset 위치로 이동 후 DirectoryEntry 하나를 읽음
// 오류 발생시 entry->inode 가 0으로 설정되고 0을 리턴
// 읽은 바이트 수를 리턴
uint64_t ext2ReadDirectoryEntry(
    FILE *fp,
    const struct Ext2Superblock *sb,
    const struct Ext2INode *node,
    uint64_t byteOffset,
    struct Ext2DirectoryEntry *entry)
{
    uint64_t blockSize = ext2GetBlockSize(sb);
    char *buf = malloc(blockSize);
    if (buf == NULL)
    {
        perror("malloc buf");
        goto returnError;
    }

    // byteOffset 위치하는 block 읽기
    uint32_t blockPointerOffset = byteOffset / blockSize;
    int64_t blockReadBytes = ext2ReadContent(fp, sb, node, blockPointerOffset, buf);
    if (blockReadBytes < 0) // 오류
    {
        goto returnError;
    }
    if (blockReadBytes == 0) // 디렉토리의 끝을 넘어감
    {
        goto returnError;
    }

    // block 에서 byteOffset 위치 찾기
    uint32_t offsetInBlock = byteOffset % blockSize;

    // 필드 복사
    uint16_t entrySize;
    uint8_t nameLength;
    memcpy(&entry->inode, buf + offsetInBlock + 0, 4);
    memcpy(&entrySize, buf + offsetInBlock + 4, 2);
    memcpy(&nameLength, buf + offsetInBlock + 6, 1);

    // 잘못된 값 읽으면 무한루프 발생하기에 예외처리 필요
    if (entrySize < 8 + nameLength) // inode + rec_len + name_len + file_type
    {
        fprintf(stderr, "too small entry size");
        goto returnError;
    }
    if (entrySize % 4 != 0)
    {
        fprintf(stderr, "entry size was not multiple of 4");
        goto returnError;
    }
    // 크기가 너무 큰 경우
    if (offsetInBlock + entrySize > (uint64_t)blockReadBytes)
    {
        fprintf(stderr, "entry size exceeded a block");
        goto returnError;
    }

    // 이름 복사, nameLength 는 8비트, entry->name 은 256 크기라서 무조건 들어감
    memcpy(entry->name, buf + offsetInBlock + 8, nameLength);
    entry->name[nameLength] = '\0';
    free(buf);
    return entrySize;

returnError:
    if (buf != NULL)
        free(buf);
    entry->inode = 0;
    return 0;
}

// 하위 노드를 링크드 리스트 형태로 반환. 호출자가 free 해야함
struct Ext2DirectoryEntryList *ext2ScanDir(
    FILE *fp,
    const struct Ext2Superblock *sb,
    struct Ext2INode *node)
{
    // 디렉토리가 맞는지 확인
    if ((node->typeAndPermission & EXT2_INODE_TYPE_MASK) != EXT2_INODE_TYPE_DIR)
    {
        return NULL;
    }

    // 링크드 리스트 초기화
    struct Ext2DirectoryEntryList *list = malloc(sizeof(struct Ext2DirectoryEntryList));
    list->next = NULL;
    struct Ext2DirectoryEntryList *lastNode = list;

    // 디렉토리 내용 읽기
    struct Ext2DirectoryEntry entry;
    uint64_t dirOffset = 0;
    uint64_t readBytes;
    while ((readBytes = ext2ReadDirectoryEntry(fp, sb, node, dirOffset, &entry)) > 0)
    {
        // 유효한 inode 일때
        if (entry.inode > 0)
        {
            // 링크드 리스트 끝에 새 노드 추가
            struct Ext2DirectoryEntryList *node = malloc(sizeof(struct Ext2DirectoryEntryList));
            node->next = NULL;
            node->entry = entry;
            lastNode->next = node;
            lastNode = node;
        }
        dirOffset += readBytes;
    }

    return list;
}

uint32_t findNextNode(
    FILE *fp,
    const struct Ext2Superblock *sb,
    uint32_t workingDirectoryNode,
    const char *segment)
{
    // workingDirectoryNode 읽기
    struct Ext2INode node;
    if (ext2ReadNode(fp, sb, workingDirectoryNode, &node) != 0)
    {
        return 0;
    }

    // workingDirectoryNode 의 내용을 링크드 리스트 형태로 가져오기
    struct Ext2DirectoryEntryList *list = ext2ScanDir(fp, sb, &node);
    if (list == NULL)
    {
        return 0;
    }
    struct Ext2DirectoryEntryList *currentNode = list->next;

    // 리스트 순회
    uint32_t foundNodeAddress = 0;
    while (currentNode != NULL)
    {
        // segment 발견!
        if (strcmp(currentNode->entry.name, segment) == 0)
        {
            foundNodeAddress = currentNode->entry.inode;
        }

        // 리스트의 다음 노드로 이동
        struct Ext2DirectoryEntryList *nextNode = currentNode->next;
        free(currentNode);
        currentNode = nextNode;
    }

    free(list);
    return foundNodeAddress;
}

// path 를 찾아서 inode 번호 반환
// path 가 상대 경로인 경우, workingDirectoryNode 부터 탐색 시작
// path 가 절대 경로인 경우, ROOT DIRECTORY 부터 탐색 시작
uint32_t ext2FindPath(
    FILE *fp,
    const struct Ext2Superblock *sb,
    uint32_t workingDirectoryNode,
    const char *path)
{
    // 빈 경로 문자열 처리
    int pathLen = strlen(path);
    if (pathLen == 0)
    {
        if (workingDirectoryNode > 0)
            return workingDirectoryNode;
        else
            return 0;
    }

    if (path[0] == '/') // 절대경로는 working directory 가 root directory
    {
        workingDirectoryNode = ROOT_DIRECTORY_INODE;
    }
    else if (workingDirectoryNode == 0) // 상대 경로인데 workingDirectory 설정되지 않음
    {
        fprintf(stderr, "Can't resolve relative paths without working directory.\n");
        return 0;
    }

    // path segment 임시로 저장할 공간
    int segmentLength = 0;
    char segmentBuf[PATH_SEGMENT_MAX_LENGTH + 1];

    // 경로 문자 순회
    for (int i = 0; i < pathLen; i++)
    {
        if (path[i] == '/' && segmentLength > 0) // segment 경계, 빈 segment 무시 (예시: /a//b///c//d/ )
        {
            // workingDirectory 에서 segment 찾기
            segmentBuf[segmentLength] = '\0';
            uint64_t result = findNextNode(
                fp,
                sb,
                workingDirectoryNode,
                segmentBuf);

            if (result > 0)
                workingDirectoryNode = result;
            else
                return 0;

            segmentLength = 0;
        }
        else
        {
            // 디렉토리 이름이 255자를 넘어서는 경우, 예외처리 필요
            if (segmentLength >= PATH_SEGMENT_MAX_LENGTH)
            {
                fprintf(stderr, "Too long path segment\n");
                return 0;
            }

            segmentBuf[segmentLength++] = path[i];
        }
    }

    // 처리되지 않은 segment 남아있다면, 한번 더 찾기
    if (segmentLength > 0)
    {
        segmentBuf[segmentLength] = '\0';
        return findNextNode(
            fp,
            sb,
            workingDirectoryNode,
            segmentBuf);
    }
    else
    {
        return workingDirectoryNode;
    }
}