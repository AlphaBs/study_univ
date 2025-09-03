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

void handler(int signo)
{
	printf("Awake!\n");
}

void my_sleep(unsigned int seconds)
{
	alarm(seconds);
	pause();
}

int main()
{
	struct sigaction sigact;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigact.sa_handler = handler;
	if (sigaction(SIGALRM, &sigact, NULL) == -1)
	{
		perror("sigaction");
		exit(1);
	}

	while (1)
	{
		int seconds;
		printf("How many seconds should I sleep? (-1 to exit): ");
		int ret = scanf("%d", &seconds);
		if (ret != 1)
		{
			printf("Input error. The program will now exit.\n");
			break;
		}
		if (seconds == -1)
		{
			printf("The program will now exit\n");
			break;
		}
		else if (seconds <= 0)
		{
			printf("Please enter a value greater than 0.\n");
			continue;
		}
		else
		{
			printf("Sleeping for %u seconds...\n", seconds);
			my_sleep(seconds);
		}
	}

	exit(0);
	return 0;
}
