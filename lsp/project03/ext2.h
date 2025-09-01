#ifndef EXT2_H
#define EXT2_H

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#define SUPERBLOCK_OFFSET 1024
#define SUPERBLOCK_SIZE 1024
#define SUPERBLOCK_MAGIC 0xEF53
#define GROUPDESC_SIZE 32
#define BLOCK_POINTER_SIZE 4
#define DIRECT_BLOCK_POINTER_COUNT 12
#define ROOT_DIRECTORY_INODE 2
#define PATH_SEGMENT_MAX_LENGTH 255
#define EXT2_INODE_TYPE_MASK 0xF000
#define EXT2_INODE_TYPE_DIR  0x4000
#define EXT2_INODE_TYPE_REG  0x8000

struct Ext2Superblock {
	uint32_t	inodeCount;
	uint32_t	blockCount;
	uint32_t	blockSizeInLog;
	uint32_t	blocksPerGroup;
	uint32_t	inodesPerGroup;
	uint16_t	magic;
    uint32_t    majorVersion;
	uint16_t    inodeSize;
};

struct Ext2GroupDesc {
    uint32_t blockBitmapBlock;
    uint32_t inodeBitmapBlock;
    uint32_t inodeTableBlock;
};

struct Ext2INode {
	uint16_t typeAndPermission;
	uint32_t sizeLowerBits;
	uint32_t directBlockPointer[12];
	uint32_t singleIndirectBlockPointer;
	uint32_t doubleIndirectBlockPointer;
	uint32_t tripleIndirectBlockPointer;
};

struct Ext2DirectoryEntry {
	uint32_t inode;
	char     name[PATH_SEGMENT_MAX_LENGTH + 1];
};

struct Ext2DirectoryEntryList {
	struct Ext2DirectoryEntry entry;
	struct Ext2DirectoryEntryList* next;
};

int ext2ReadSuperblock(FILE* fp, struct Ext2Superblock* sb);
void ext2PrintSuperblock(const struct Ext2Superblock* sb);
uint64_t ext2GetBlockSize(const struct Ext2Superblock *sb);
int ext2ReadNode(FILE* fp, const struct Ext2Superblock *sb, uint32_t inodeAddress, struct Ext2INode *node);
void ext2PrintNode(const struct Ext2INode* node);

int ext2ReadBlock(
	FILE *fp, 
	const struct Ext2Superblock *sb, 
	uint32_t blockAddress,
	char *buf,
	uint32_t bufSize);

int64_t ext2ReadContent(
	FILE *fp, 
	const struct Ext2Superblock *sb, 
	const struct Ext2INode *node, 
	uint32_t blockPointerOffset,
	char* buf);

uint64_t ext2ReadDirectoryEntry(
    FILE *fp,
    const struct Ext2Superblock *sb,
    const struct Ext2INode *node,
    uint64_t byteOffset,
    struct Ext2DirectoryEntry *entry);

struct Ext2DirectoryEntryList* ext2ScanDir(
	FILE* fp, 
	const struct Ext2Superblock *sb, 
	struct Ext2INode* node);

uint32_t ext2FindPath(
    FILE *fp, 
    const struct Ext2Superblock *sb, 
    uint32_t workingDirectoryNode, 
    const char *path);

#endif