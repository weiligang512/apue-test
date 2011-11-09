#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

void *fun(void *arg);
void routine(void *arg);

int main(int argc, char **argv)
{
#if 0
	void pthread_cleanup_pop(int execute);
	void pthread_cleanup_push(void (*routine)(void*), void *arg)
	
#endif
	
	int statue, i;
	pthread_t thread[5];
	char *p = "cleanup";

	pthread_cleanup_push(routine, (void *)0);


	statue = pthread_create(&thread[i], NULL, fun, (void *)&i);

	pthread_cleanup_pop(0);
	if (statue != 0)
	{
		fprintf(stderr, "create pthread failed %s\n", strerror(statue));
	}
	}

//	exit(0);
	pthread_join(thread[0], (void **)1);
	return 0;
}


void *fun(void *arg)
{
	int i;
	fprintf(stdout, "arg = %d\n", *(int *)arg);
	for (i = 0; i < 10; i++)
	{
		fprintf(stdout, "in pthread i = %d\n", i);
		fprintf(stdout, "->");
		fflush(stdout);
		sleep(1);
	}
	

	return (void *)0;
}

void routine(void *arg)
{
	//printf("in roution:%s\n", (char *)arg);
	printf("in roution: %d\n", (int)arg);
}
