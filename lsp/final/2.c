#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

int main(void)
{
	int fd;
	int val;

	fd = open("ssu_test.txt", O_RDONLY);
	if (fd < 0)
	{
		perror("open");
		exit(1);
	}

	val = fcntl(fd, F_GETFD, 0);
	if (val & FD_CLOEXEC)
	{
		printf("close-on-exec bit on\n");
	}
	else
	{
		printf("close-on-exec bit off\n");
	}

	val |= FD_CLOEXEC;

	if (fcntl(fd, F_SETFD, val) < 0)
	{
		perror("fcntl");
		exit(1);
	}

	printf("close-on-exec bit on\n");
	execl("/home/ksi/lsp/final/ssu_loop.o", "./ssu_loop.o", NULL);
	exit(0);
	return 0;
}
