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
#include <wait.h>

pid_t pid;

void handler(int signo)
{
	int stat;
	if (waitpid(pid, &stat, 0) != -1)
	{
		perror("waitpid");
		exit(1);
	}

	printf("Child terminated with status: %d\n", stat);

	if (WIFEXITED(stat))
		printf("normal termination, exit status = %d\n", WEXITSTATUS(stat));
	else if (WIFSIGNALED(stat))
		printf("abnormal signal, signo = %d\n", WTERMSIG(stat));
	else if (WIFSTOPPED(stat))
		printf("stopped, signo = %d\n", WSTOPSIG(stat));

	exit(0);
}

int main()
{
	pid = fork();
	if (pid < 0)
	{
		perror("fork");
		exit(1);
	}
	else if (pid == 0) // child
	{
		printf("Child PID: %d, executing...\n", getpid());
		char *list[] = { "ls", "/", NULL };
		execlp("/bin/ls", "ls", "/", NULL);
		exit(0);
	}
	else // parent
	{
		printf("Parent PID: %d\nParent UID: %d\n", getpid(), getuid());

		struct sigaction sigact;
		sigemptyset(&sigact.sa_mask);
		sigact.sa_flags = 0;
		sigact.sa_handler = handler;
		if (sigaction(SIGCHLD, &sigact, NULL) != 0)
		{
			perror("sigaction");
			exit(1);
		}

		int stat;
		if (wait(&stat) == -1)
		{
			perror("wait");
			exit(1);
		}
		pause();
	}

	exit(0);
	return 0;
}
