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

static void ssu_signal_handler1(int signo);
static void ssu_signal_handler2(int signo);

int main(void)
{
	struct sigaction sigact;
	sigemptyset(&sigact.sa_mask);
	sigaddset(&sigact.sa_mask, SIGQUIT);
	sigact.sa_handler = ssu_signal_handler1;
	if (sigaction(SIGINT, &sigact, NULL) == -1)
	{
		perror("sigint");
		exit(1);
	}

	struct sigaction sigquit;
	sigemptyset(&sigquit.sa_mask);
	sigaddset(&sigquit.sa_mask, SIGINT);
	sigquit.sa_handler = ssu_signal_handler2;
	if (sigaction(SIGQUIT, &sigquit, NULL) == -1)
	{
		perror("sigquit");
		exit(1);
	}

	pause();
	exit(0);
	return 0;
}

static void ssu_signal_handler1(int signo)
{
	printf("signal handler of SIGINT: %d\n", signo);
	printf("sigquit signal is blocked: %d\n", signo);
	printf("sleeping 3 sec\n");
	sleep(3);
	printf("signal handler of sigint ended\n");
}

static void ssu_signal_handler2(int signo)
{
	printf("signal handler of sigquit : %d\n", signo);
	printf("sigint signal is blocked: %d\n", signo);
	printf("sleeping 3 sec\n");
	sleep(3);
	printf("signal handler of sigint ended\n");
}
