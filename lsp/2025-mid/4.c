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

int main() {
	FILE *fp;
	int ch;
	char word[128];
	int index = 0;

	fp = fopen("ssu_words.txt", "r");
	if (fp == NULL) {
		fprintf(stderr, "fopen error\n");
		exit(1);
	}
	
	bool written = false;
	while ((ch = fgetc(fp)) != EOF) {
		if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z')) {
			word[index++] = ch;
			written = true;
		}
		else {
			word[index] = '\0';
			if (written)
				printf("%s\n", word);
			printf("Separator => %c\n", ch);
			index = 0;
			written = false;
		}
	}

	if (fclose(fp) == EOF) {
		fprintf(stderr, "fclose error\n");
		exit(1);
	}
	exit(0);
}
