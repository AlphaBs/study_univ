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
#include <pthread.h>

pthread_mutex_t mutexcond = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexcount = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int input = 0;
int count = 0;
int t1 = 0;
int t2 = 1;

void* pib1(void *arg)
{
	while (1)
	{
		pthread_mutex_lock(&mutexcount);

		if (count >= input)
		{
			pthread_mutex_unlock(&mutexcount);
			break;
		}

		if (count > 0)
			pthread_cond_wait(&cond, &mutexcount);

		if (count % 2 == 0)
		{
			printf("Thread 1 : %d\n", t1);
			t1 = t1 + t2;
			count++;
		}

		pthread_mutex_unlock(&mutexcount);
		pthread_cond_signal(&cond);
	}
	return NULL;
}

void* pib2(void *arg)
{
	while (1)
	{
		pthread_mutex_lock(&mutexcount);

		if (count >= input)
		{
			pthread_mutex_unlock(&mutexcount);
			break;
		}

		if (count > 1)
			pthread_cond_wait(&cond, &mutexcount);

		if (count % 2 == 1)
		{
			printf("Thread 2 : %d\n", t2);
			t2 = t2 + t1;
			count++;
		}

		pthread_mutex_unlock(&mutexcount);
		pthread_cond_signal(&cond);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	while (1)
	{
		printf("2ro dltkddml rown dlqfur : ");
		scanf("%d", &input);
		if (input >= 2) break;
	}

	pthread_t th1;
	pthread_t th2;

	if (pthread_create(&th1, NULL, pib1, NULL) != 0)
	{
		perror("thread 1");
		exit(1);
	}

	if (pthread_create(&th2, NULL, pib2, NULL) != 0)
	{
		perror("thread 2");
		exit(1);
	}

	pthread_join(th1, NULL);
	pthread_join(th2, NULL);
	pthread_mutex_destroy(&mutexcount);
	pthread_mutex_destroy(&mutexcond);
	pthread_cond_destroy(&cond);

	printf("complete\n");

	return 0;
}
