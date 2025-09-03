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

#define MODE_EXEC ( S_IXUSR | S_IXGRP | S_IXOTH )

int main(int argc, char *argv[]) {
	struct stat statbuf;
	int i;

	if (argc < 2) {
		fprintf(stderr, "wrong usage!\n");
		exit(1);
	}

	for (i = 1; i < argc; i++) {
		if (stat(argv[i], &statbuf) < 0) {
			fprintf(stderr, "stat error %s\n", argv[i]);
			continue;
		}

		mode_t newMode = (statbuf.st_mode | MODE_EXEC);
		newMode ^= S_IXGRP;
		newMode ^= S_IXOTH;
		if (chmod(argv[i], newMode) < 0) {
			fprintf(stderr, "chmod error %s\n", argv[i]);
			continue;
		}

		printf("%s : file permission was changed.\n", argv[i]);
	}

	exit(0);
}
