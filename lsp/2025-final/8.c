#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>

int main(int argc, char *argv[])
{
	struct timespec start, end;
	long seconds, nanoseconds;

	clock_gettime(CLOCK_MONOTONIC, &start);

	if (argc != 2)
	{
		fprintf(stderr, "wrong usage\n");
		exit(1);
	}

	int fd = open(argv[1], O_RDWR);
	if (fd < 0)
	{
		perror("open");
		exit(1);
	}

	struct stat statbuf;
	if (fstat(fd, &statbuf) == -1)
	{
		perror("fstat");
		exit(1);
	}
	off_t size = statbuf.st_size;

	if (size < 16)
	{
		fprintf(stderr, "file size should be exceed 16 bytes. (%ld) bytes\n", size);
		exit(1);
	}

	void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED)
	{
		perror("mmap");
		exit(1);
	}
	printf("Original: ");
	fputs(addr, stdout);

	int upperPos = -1;
	for (int i = 0; i < 8; i++)
	{
		char ch = *((char*)(addr + i));
		if ('A' <= ch && ch <= 'Z')
		{
			upperPos = i;
			break;
		}
	}

	if (upperPos == -1)
	{
		fprintf(stderr, "no uppercase in the file\n");
		exit(1);
	}

	memmove(addr + upperPos + 3, addr + upperPos, 5);
	printf("Moved 5 bytes from index %d to index %d\n", upperPos, upperPos + 3);
	//fputs(addr, stdout); // DEBUG

	int halfPos = size / 2;
	int halfLen = size - halfPos;

	char* buf = malloc(halfLen);
	memcpy(buf, addr + halfPos, halfLen);
	for (int i = 0; i < halfLen; i++)
	{
		char minCh = buf[i];
		int minIdx = i;
		for (int j = i + 1; j < halfLen; j++)
		{
			if (buf[j] < minCh)
			{
				minCh = buf[j];
				minIdx = j;
			}
		}

		// swap buf[i] and buf[minIdx]
		char temp = buf[i];
		buf[i] = buf[minIdx];
		buf[minIdx] = temp;
	}
	memcpy(addr + halfPos, buf, halfLen);
	printf("Sorted back-half.\n");
	//fputs(addr, stdout); // DEBUG

	if (mprotect(addr, size, PROT_READ) != 0)
	{
		perror("mprotect 1");
	}

	if (mprotect(addr, size, PROT_READ | PROT_WRITE) != 0)
	{
		perror("mprotect 2");
	}

	printf("Final 4 bytes set to X, Y, Z, !\n");
	memset(addr + size - 4, 'X', 1);
	memset(addr + size - 3, 'Y', 1);
	memset(addr + size - 2, 'Z', 1);
	memset(addr + size - 1, '!', 1);

	printf("Final file cntent: ");
	fputs(addr, stdout);

	if (msync(addr, size, MS_SYNC) != 0)
	{
		perror("msync");
		exit(1);
	}

	munmap(addr, size);

	//sleep(1);
	clock_gettime(CLOCK_MONOTONIC, &end);
	seconds = end.tv_sec - start.tv_sec;
	nanoseconds = end.tv_nsec - start.tv_nsec;
	if (nanoseconds < 0)
	{
		seconds--;
		nanoseconds += 1e9;
	}

	double elapsedSeconds = seconds + nanoseconds * 1e-9;
	printf("Execution time: %f microseconds\n", elapsedSeconds * 1000 * 1000);
	exit(0);
	return 0;
}
