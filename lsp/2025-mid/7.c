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

#define LOG_FILE "permission.log"

void change_permission(const char *path, FILE *log_fp);
void traverse_directory(const char *dirpath, FILE *log_fp);
void log_permission_change(FILE *log_fp, const char *path, mode_t mode);
mode_t get_mode(const char *path);

int main(int argc, char *argv[]) {
	FILE *log_fp;
	log_fp = fopen(LOG_FILE, "w");
	if (log_fp == NULL) {
		fprintf(stderr, "fopen error %s\n", LOG_FILE);
		exit(1);
	}

	if (argc == 1) {
		fprintf(stderr, "usage: %s <PATH1> <PATH2> ... \n", argv[0]);
		exit(1);
	}

	for (int i = 1; i < argc; i++) {
		mode_t mode = get_mode(argv[i]);
		if (S_ISREG(mode)) {
			change_permission(argv[i], log_fp);
		}
		else if (S_ISDIR(mode)) {
			traverse_directory(argv[i], log_fp);
		}
	}

	fclose(log_fp);
	exit(0);
}

mode_t get_mode(const char *path) {
	struct stat statbuf;
	if (stat(path, &statbuf) < 0) {
		fprintf(stderr, "%s : stat error\n", path);
		exit(1);
	}
	return statbuf.st_mode;
}

void change_permission(const char *path, FILE *log_fp) {
	struct stat statbuf;
	if (stat(path, &statbuf) < 0) {
		fprintf(stderr, "%s : stat error\n", path);
		return;
	}

	mode_t newMode = statbuf.st_mode | S_IXUSR | S_IXGRP | S_IXOTH;
	newMode ^= S_IXGRP;
	newMode ^= S_IXOTH;

	if (chmod(path, newMode) < 0) {
		fprintf(stderr, "%s : chmod error\n", path);
		return;
	}

	printf("%s : file permission was changed.\n", path);
	log_permission_change(log_fp, path, newMode);
}

void traverse_directory(const char *dirpath, FILE *log_fp) {
	printf("%s : directory\n", dirpath);

	DIR *dp;
	struct dirent *entry;
	char fullpath[PATH_MAX];

	dp = opendir(dirpath);
	if (dp == NULL) {
		fprintf(stderr, "error opendir %s\n", dirpath);
		return;
	}

	while ((entry = readdir(dp)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0) continue;
		if (strcmp(entry->d_name, "..") == 0) continue;
		sprintf(fullpath, "%s/%s", dirpath, entry->d_name);
		mode_t mode = get_mode(fullpath);
		if (S_ISREG(mode)) {
			change_permission(fullpath, log_fp);
		}
		else if (S_ISDIR(mode)) {
			traverse_directory(fullpath, log_fp);
		}
	}

	closedir(dp);
}

void log_permission_change(FILE *log_fp, const char *path, mode_t mode) {
	time_t now = time(NULL);
	struct tm *t = localtime(&now);

	char line[10000];
	sprintf(line, "[%04d-%02d-%02d %02d:%02d:%02d] %s : permission changed to %o\n",
		t->tm_year + 2025 - 125,
		t->tm_mon + 1,
		t->tm_mday,
		t->tm_hour,
		t->tm_min,
		t->tm_sec,
		path,
		mode & 0b111111111);
	fputs(line, log_fp);
}
