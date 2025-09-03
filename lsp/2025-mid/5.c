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

int main(int argc, char *argv[]) 
{
	struct stat file_info;
	char *str;
	int i;

	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);

	for (i = 1; i < argc; i++) {
		if (lstat(argv[i], &file_info) < 0) {
			fprintf(stderr, "lstat error for %s\n", argv[i]);
			continue;
		}


		if (S_ISREG(file_info.st_mode)) {
			str = "regular";
		}
		else if (S_ISDIR(file_info.st_mode)) {
			str = "directory";
		}
		else if (S_ISCHR(file_info.st_mode)) {
			str = "character special";
		}
		else if (S_ISBLK(file_info.st_mode)) {
			str = "block special file";
		}
		else if (S_ISFIFO(file_info.st_mode)) {
			str = "FIFO";
		}
		else if (S_ISLNK(file_info.st_mode)) {
			str = "symbolic link";
		}
		else if (S_ISSOCK(file_info.st_mode)) {
			str = "socket";
		}
		printf("name = %s, type = %s\n", argv[i], str);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);

	long seconds = end.tv_sec - start.tv_sec;
	long nanoseconds = end.tv_nsec - start.tv_nsec;
	double elapsed;
	if (nanoseconds < 0) {
		seconds--;
		nanoseconds += 1e9;
	}

	elapsed = seconds + nanoseconds * 1e-9;
	printf("Elapsed time: %.9f seconds\n", elapsed);

	exit(0);
}
