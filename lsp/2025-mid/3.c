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

#define BUFFER_SIZE 1024

int main() {
	char buf[BUFFER_SIZE];
	ssize_t num_read;

	num_read = read(0, buf, BUFFER_SIZE - 1);
	if (num_read < 0) {
		fprintf(stderr, "read errror\n");
		exit(1);
	}
	buf[num_read] = '\0';
	printf("%s", buf);

	exit(0);
}
