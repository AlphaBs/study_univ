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

int main(void) {
	char buf[BUFFER_SIZE];
	int a, b;
	int i;

	setbuf(stdin, buf);
	scanf("%d %d", &a, &b);

	for (i = 0; buf[i] != '\n'; i++)
		putchar(buf[i]);
	putchar('\n');

	exit(0);
}
