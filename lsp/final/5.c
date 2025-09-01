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
#include <errno.h>

void set_flags(int fd, int flags);
void clr_flags(int fd, int flags);
char buf[500000];

int main(void)
{
	int ntowrite, nwrite;
	char *ptr;

	ntowrite = read(STDIN_FILENO, buf, sizeof(buf));
	ptr = buf;
	fprintf(stderr, "reading %d bytes\n", ntowrite);
	set_flags(STDOUT_FILENO, O_NONBLOCK);

	while (ntowrite > 0)
	{
		errno = 0;
		nwrite = write(STDOUT_FILENO, buf, ntowrite);
		fprintf(stderr, "write %d, error %d", nwrite, errno);
		if (nwrite > 0)
		{
			ntowrite -= nwrite;
			ptr += nwrite;
		}
		fprintf(stderr, "remain %d\n", ntowrite);
	}

	set_flags(STDOUT_FILENO, O_NONBLOCK);

	exit(0);
	return 0;
}

void set_flags(int fd, int flags)
{
	int val = fcntl(fd, F_GETFL, 0);
	if (val < 0)
	{
		perror("fcntl");
		exit(1);
	}

	val |= flags;
	fcntl(fd, F_SETFL, val);
}

void clr_flags(int fd, int flags)
{
	int val = fcntl(fd, F_GETFL, 0);
	if (val < 0)
	{
		perror("fcntl");
		exit(1);
	}

	val &= ~flags;
	fcntl(fd, F_SETFL, val);
}
