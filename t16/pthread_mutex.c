#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>


int global_value = 0;
pthread_mutex_t mutex;
pthread_cond_t cond;

void *fun(void *arg);

int main(int argc, char **argv)
{
//int pthread_mutex_destroy(pthread_mutex_t *mutex);
//int pthread_mutex_init(pthread_mutex_t *restrict mutex,
//const pthread_mutexattr_t *restrict attr);
//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_init(&mutex, NULL);
//int pthread_mutex_lock(pthread_mutex_t *mutex);
//int pthread_mutex_trylock(pthread_mutex_t *mutex);
//int pthread_mutex_unlock(pthread_mutex_t *mutex);
	pthread_cond_init(&cond, NULL);			                     
//int pthread_cond_destroy(pthread_cond_t *cond);
//int pthread_cond_init(pthread_cond_t *restrict cond, const pthread_condattr_t *restrict attr);

	pthread_t pthread;
	pthread_create(&pthread, NULL, fun, (void *)0);

	for(;;)
	{
		pthread_mutex_lock(&mutex);
		global_value++;
		printf("int main:globalvalue = %d\n", global_value);
		if (global_value % 2 == 0)
			pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
		sleep(1);
	}

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);

	pthread_exit((void *)0);
}

void *fun(void *arg)
{
	for(;;)
	{
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&cond, &mutex);
		printf("int fun:globalvbal = %d\n", global_value);
		pthread_mutex_unlock(&mutex);
	}
	return (void *)0;
}

