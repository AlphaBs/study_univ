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
	printf("in ssu_signal_handler function\n");
}

void check()
{
	sigset_t sigset;
	if (sigpending(&sigset) == -1)
	{
		perror("sigpending");
		exit(1);
	}

	if (sigismember(&sigset, SIGUSR1))
	{
		printf("a SIGUSR1 signal is pending\n");
	}
	else
	{
		printf("SIGUSR1 signals are not pending\n");
	}
}

int main(int argc, char *argv[])
{
	struct sigaction sigact;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigact.sa_handler = handler;
	if (sigaction(SIGUSR1, &sigact, NULL) == -1)
	{
		perror("sigaction SIGUSR1");
		exit(1);
	}

	sigset_t oldset;
	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGUSR1);
	if (sigprocmask(SIG_BLOCK, &sigset, &oldset) == -1)
	{
		perror("sigprocmask");
		exit(1);
	}
	printf("SIGUSR1 signals are now blocked\n");

	kill(getpid(), SIGUSR1);
	printf("after kill()\n");
	check();

	if (sigprocmask(SIG_SETMASK, &oldset, NULL) == -1)
	{
		perror("sigprocmask");
		exit(1);
	}
	printf("SIGUSR1 signals are no longer blocked\n");

	check();
	return 0;
}
