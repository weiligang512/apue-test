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
	int statue;

	statue = pthread_create(&pthread, NULL, fun, (void *)0);
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
	fprintf(stdout, "in fun:%08x\n", (int)pthread_self());
	int i;
	pthread_t pthread;
	int statue;

	char *pp = "from fun";


	statue = pthread_create(&pthread, NULL, fun1, (void *)pp);
	if (statue != 0)
	{
		fprintf(stderr, "create pthread failed %s\n", strerror(statue));
	}

	for(i = 0;i < 0; i++)
	{
		printf(".");
		fflush(stdout);
		sleep(1);
	}
#if 0
	if (pthread_cancel(pthread) != 0)
	{
		printf("cancel pthread %08x failed\n", (int)pthread);
	}
#endif
	char *p = (void *)89;
	//int pthread_join(pthread_t thread, void **value_ptr);	
	pthread_join(pthread, (void *)&p);
	printf("in fun:%s\n", (char *)p);

	pthread_exit((void *)0);
}

void *fun1(void *arg)
{
	fprintf(stdout, "in fun1:%08x\n", (int)pthread_self());
//	fprintf(stdout, "int fun1:%s\n", (char *)arg);
	int i;
	char *p = "returned fun1";

	for(i = 0;i < 5; i++)
	{
		printf("->");
		fflush(stdout);
		sleep(1);
	}

	pthread_exit((void *)p);
}
