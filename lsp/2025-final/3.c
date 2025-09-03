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

struct employee
{
	char name[50];
	int salary;
	int pid;
};

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "wrong usage\n");
		return 1;
	}

	int fd = open(argv[1], O_RDWR);
	if (fd == -1)
	{
		perror("open");
		return 1;
	}

	while (1)
	{
		printf("Enter record number: ");
		int recnum;
		scanf("%d", &recnum);
		if (recnum < 0)
			break;

		struct employee emp;
		int pos = recnum * sizeof(emp);
		
		struct flock lock;
		lock.l_type = F_RDLCK;
		lock.l_whence = 0;
		lock.l_start = pos;
		lock.l_len = sizeof(emp);

		if (fcntl(fd, F_SETLKW, &lock) == -1)
		{
			fprintf(stderr, "failed to set lock!\n");
			continue;
		}

		lseek(fd, pos, SEEK_SET);
		if (read(fd, &emp, sizeof(emp)) <= 0)
		{
			lock.l_type = F_UNLCK;
			fcntl(fd, F_SETLKW, &lock);
			fprintf(stderr, "failed to read %d. it may not exists.\n", recnum);
			continue;
		}

		printf("Employee: %s, salary: %d\n", emp.name, emp.salary);

		char line[256];
		while (1)
		{
			printf("Do you want to update salary (y or n)? ");
			scanf("%s", line);

			if (line[0] == 'y' || line[0] == 'n')
				break;
		}

		if (line[0] == 'y')
		{
			lock.l_type = F_WRLCK;
			if (fcntl(fd, F_SETLKW, &lock) == -1)
			{
				perror("fctnl F_WRLCK");
			}
			else
			{
				printf("Enter new salary: ");
				scanf("%d", &emp.salary);
				lseek(fd, pos, SEEK_SET);
				if (write(fd, &emp, sizeof(emp)) != sizeof(emp))
				{
					fprintf(stderr, "write failed");
				}
			}
		}

		lock.l_type = F_UNLCK;
		if (fcntl(fd, F_SETLKW, &lock) == -1)
		{
			perror("fcntl F_UNLCK");
		}
	}

	printf("Exiting...\n");
	close(fd);
	exit(0);
	return 0;
}
