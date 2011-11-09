#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

void fun(void *arg);

int main(int argc, char **argv)
{
	int i;
//	void pthread_cleanup_pop(int execute);
//	void pthread_cleanup_push(void (*routine)(void*), void *arg);
	pthread_cleanup_push(fun, (void *)0);
	pthread_cleanup_push(fun, (void *)1);
	

	for(i = 0; i < 5; i++)
	{
		printf("->");
		fflush(stdout);
		sleep(1);
	}

	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);

	pthread_exit((void *)0);

	puts("can I go to here?");
}


void fun(void *arg)
{
	static int i = 1;
	printf("i = %d\n", i);
	fprintf(stdout, "int fun: %d\n", (int)arg);
	i++;

}
