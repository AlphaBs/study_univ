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

#define NAME_SIZE 256

int main(void) {
	FILE *fp1, *fp2, *fp3;
	char buf1[NAME_SIZE];
	char buf2[NAME_SIZE];
	char fname1[NAME_SIZE];
	char fname2[NAME_SIZE];
	char temp[] = "temp_merge.txt";

	printf("Enter your first file name: ");
	scanf("%s", fname1);

	printf("Enter your second file name: ");
	scanf("%s", fname2);

	fp1 = fopen(fname1, "r");
	if (fp1 == NULL) {
		fprintf(stderr, "fopen error %s\n", fname1);
		exit(1);
	}

	fp2 = fopen(fname2, "r");
	if (fp2 == NULL) {
		fprintf(stderr, "fopen error %s\n", fname2);
		exit(1);
	}

	fp3 = fopen(temp, "w");
	if (fp3 == NULL) {
		fprintf(stderr, "fopen error %s\n", temp);
		exit(1);
	}

	int line_number = 1;
	while (true) {
		char* result1 = fgets(buf1, NAME_SIZE, fp1);
		char* result2 = fgets(buf2, NAME_SIZE, fp2);

		if (result1 == NULL && result2 == NULL) {
			break;
		}
		if (result1 != NULL) {
			int len1 = strlen(result1);
			if (len1 == 0)
				continue;
			fprintf(fp3, "%d: %s", line_number, buf1);
			line_number++;
		}
		if (result2 != NULL) {
			int len2 = strlen(result2);
			if (len2 == 0)
				continue;
			fprintf(fp3, "%d: %s", line_number, buf2);
			line_number++;	
		}
	}
	
	fclose(fp1);
	fclose(fp2);
	fclose(fp3);

	if (remove(fname1) != 0) {
		fprintf(stderr, "error remove %s\n", fname1);
		exit(1);
	}
	if (rename(temp, fname1) != 0) {
		fprintf(stderr, "error rename %s %s\n", temp, fname1);
		exit(1);
	}

	exit(0);
}
