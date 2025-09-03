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
#include <syslog.h>

char* programName;

void start_loop()
{
	pid_t pid = getpid();
	openlog(programName, LOG_PID | LOG_CONS, LOG_DAEMON);
	syslog(LOG_INFO, "Daemon (PID %d) is alive", pid);
	sleep(5);
	syslog(LOG_INFO, "Daemon (PID %d) is alive", pid);
	sleep(5);
	syslog(LOG_INFO, "Daemon (PID %d) is alive", pid);
	closelog();
}

void daemon_init()
{
	pid_t pid = fork();
	if (pid < 0)
	{
		perror("fork");
		exit(1);
	}
	if (pid > 0)
	{
		exit(0);
	}

	setsid();
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	int fd;
	for (fd = 0; fd < getdtablesize(); fd++)
	{
		close(fd);
	}
	umask(0);
	chdir("/");
	fd = open("/dev/null", O_RDWR);
	dup(0);
	dup(0);

	start_loop();
}

int main(int argc, char *argv[])
{
	programName = argv[0];
	printf("parent PID: %d\n", getpid());
	daemon_init();
	return 0;
}
