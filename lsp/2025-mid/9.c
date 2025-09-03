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
#include <limits.h>
#define _GNU_SOURCE

#ifdef PATH_MAX
static int pathmax = PATH_MAX;
#else
static int pathmax = 0;
#endif
#define MAX_PATH_GUESSED 1024

#ifndef LINE_MAX
#define LINE_MAX 2048
#endif

char *pathname;
char command[LINE_MAX * 2], grep_cmd[LINE_MAX];

int ssu_do_grep(void) {
	//printf("%s\n", grep_cmd);
	return system(command);
}

void ssu_make_grep(int argc, char *argv[]) {
	strcpy(command, grep_cmd);
	for (int i = 0; i < argc; i++) {
		strcat(command, argv[i]);
		strcat(command, " ");
	}
	strcat(command, pathname);
}

void traversal(char *path, int argc, char *argv[]) {
	DIR *dp;
	dp = opendir(path);
	if (dp == NULL) {
		fprintf(stderr, "opendir error %s\n", path);
		exit(1);
	}

	struct dirent* entry;
	while ((entry = readdir(dp)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0) continue;
		if (strcmp(entry->d_name, "..") == 0) continue;

		char catpath[pathmax];
		char fullpath[pathmax];

		sprintf(catpath, "%s/%s", path, entry->d_name);
		if (realpath(catpath, fullpath) == NULL) {
			fprintf(stderr, "realpath error %s\n", catpath);
			exit(1);
		}

		struct stat statbuf;
		if (lstat(fullpath, &statbuf) < 0) {
			fprintf(stderr, "lstat error %s\n", fullpath);
			exit(1);
		}

		if (S_ISDIR(statbuf.st_mode)) {
			traversal(fullpath, argc, argv);
		}
		else if (S_ISREG(statbuf.st_mode)) {
			printf("%s : \n", fullpath);
			pathname = fullpath;
			ssu_make_grep(argc, argv);
			ssu_do_grep();
		}
	}

	closedir(dp);
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "wrong usage\n");
		exit(1);
	}

	strcpy(grep_cmd, "grep ");
	traversal(argv[argc - 1], argc - 2, &argv[1]);
	exit(0);
}
