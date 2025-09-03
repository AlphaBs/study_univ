#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <utime.h>
#include <setjmp.h>
#define _GNU_SOURCE

#define TABLE_SIZE 128
#define BUFFER_SIZE 1024

typedef struct Table {
	long offset;
	long length;
} Table;

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "wrong usage\n");
		exit(1);
	}

	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "open error %s\n", argv[1]);
		exit(1);
	}

	int i;
	Table table[TABLE_SIZE];
	table[0].offset = 0;
	table[0].length = 0;
	int entry = 0;
	int length = 0;
	char buf[BUFFER_SIZE];
	while ((length = read(fd, buf, BUFFER_SIZE)) > 0) {
		for (i = 0; i < length; i++) {
			if (buf[i] == '\n') {
				table[entry].length = i;
				table[entry + 1].offset = table[entry].offset + i + 1;
				entry++;

				if (lseek(fd, table[entry].offset, SEEK_SET) < 0) {
					fprintf(stderr, "lseek error\n");
					exit(1);
				}

				break;
			}
		}
	}

#ifdef DEBUG
	for (i = 0; i < entry; i++)
		printf("%d : %ld, %d\n", i + 1, table[i].offset, table[i].length);
#endif

	while (true) {
		printf("Enter line number : ");
		int line;
		scanf("%d", &line);
		if (line == -1) break;
		line--;
		if (lseek(fd, table[line].offset, SEEK_SET) < 0) {
			fprintf(stderr, "lseek error\n");
			exit(1);
		}
		if (read(fd, buf, BUFFER_SIZE) < 0) {
			fprintf(stderr, "read error\n");
			exit(1);
		}
		buf[table[line].length] = '\0';
		printf("%s\n", buf);
	}

	close(fd);
	exit(0);
}
