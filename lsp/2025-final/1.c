#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

int alarmCount = 1;

void handler(int signo)
{
	printf("Tick [%d]: waiting for input...\n", alarmCount);
	alarmCount++;
	alarm(5);
}

int main()
{
	printf("System monitoring started (PID: %d)\n", getpid());

	struct sigaction sigact;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigact.sa_handler = handler;
	if (sigaction(SIGALRM, &sigact, NULL) != 0)
	{
		perror("sigaction SIGALRM");
		exit(1);
	}
	handler(SIGALRM);
	
	char line[256];
	while (1)
	{
		pause();
		errno = 0;
		char* result = fgets(line, 256, stdin);
		if (result != NULL)
		{
			printf("Input received: %s", line);
			break;
		}
		if (errno == EINTR)
		{
			continue;
		}

	}
	exit(0);
	return 0;
}
