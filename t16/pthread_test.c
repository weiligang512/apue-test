#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>


void *fun(void *arg);
void *fun1(void *arg);

int main(int argc, char **argv)
{
	//int pthread_create(pthread_t *restrict thread,const pthread_attr_t *restrict attr,
	//void *(*start_routine)(void*), void *restrict arg);

	pthread_t pthread;
	pthread_t pthread1;
	int statue;
	int val = 0;
	statue = pthread_create(&pthread, NULL, fun, (void *)&val);
	if (statue != 0)
	{
		fprintf(stderr, "create pthread failed %s\n", strerror(statue));
	}
	statue = pthread_create(&pthread1, NULL, fun, (void *)&val);
	if (statue != 0)
	{
		fprintf(stderr, "create pthread failed %s\n", strerror(statue));
	}
	
	//int pthread_attr_destroy(pthread_attr_t *attr);
	//int pthread_cancel(pthread_t thread);

       	pthread_exit((void *)0);
}


void *fun(void *arg)
{
	fprintf(stdout, "in fun1:%08x\n", (int)pthread_self());
//	fprintf(stdout, "int fun1:%s\n", (char *)arg);
	int i;
	int *p = (int *)arg;

	for(*p = 0; *p < 5; (*p)++)
	{
		printf("*p = %d\n", *p);
		printf("->");
		fflush(stdout);
		sleep(1);
	}


	pthread_exit((void *)p);
}
