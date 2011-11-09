#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

void *fun(void *arg);

int main(int argc, char **argv)
{
#if 0
int pthread_create(pthread_t *restrict thread,
const pthread_attr_t *restrict attr,void *(*start_routine)(void*), void *restrict arg);

#endif 
	int statue, i;
	pthread_t thread[5];

	for (i = 0; i < 1; i++)
	{
	statue = pthread_create(&thread[i], NULL, fun, (void *)&i);

	if (statue != 0)
	{
		fprintf(stderr, "create pthread failed %s\n", strerror(statue));
	}
	//sleep(1);
	}
	//sleep(1000)
	for (i = 0; i < 5; i++)
	{
		fprintf(stdout, ".");
		fflush(stdout);
		sleep(1);
	}
	//int pthread_join(pthread_t thread, void **value_ptr);
//	pthread_exit(&thread[0]);
	exit(0);
//	pthread_join(thread[0], (void **)1);
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
