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

void appendlog()
{
	FILE *fp;
	if ((fp = fopen("/var/log/system.log", "a+")) == NULL)
	{
		perror("fopen");
		exit(1);
	}

	pid_t pid = getpid();
	fprintf(fp, "My pid is %d\n", pid);
	fclose(fp);
}

int main(void)
{
	appendlog();
	return 0;

	pid_t pid;
	pid = fork();
	if (pid < 0)
	{
		perror("fork");
		exit(1);
	}
	else if (pid > 0)
	{
		exit(0);
	}

	setsid();
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	for (int fd = 0; fd < getdtablesize(); fd++)
		close(fd);
	umask(0);
	chdir("/");
	open("/dev/null", O_RDWR);
	dup(0);
	dup(0);

	return 0;
}
