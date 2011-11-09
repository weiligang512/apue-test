#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>


int global_value = 0;
pthread_mutex_t mutex;
pthread_cond_t cond;
pthread_rwlock_t rwlock;

void *fun(void *arg);
void *fun1(void *arg);

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
//pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
//int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);
//int pthread_rwlock_init(pthread_rwlock_t *restrict rwlock,
//const pthread_rwlockattr_t *restrict attr);
	pthread_rwlock_init(&rwlock, NULL);

	pthread_t pthread;
	pthread_create(&pthread, NULL, fun, (void *)0);
	pthread_t pthread1;
	pthread_create(&pthread1, NULL, fun1, (void *)0);

	for(;;)
	{			                     
		pthread_rwlock_wrlock(&rwlock);
		global_value++;
		printf("int main:globalvalue = %d\n", global_value);
		//if (global_value % 2 == 0)
		//	pthread_cond_signal(&cond);

		pthread_rwlock_unlock(&rwlock);
		sleep(1);
	}

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
	pthread_rwlock_destroy(&rwlock);

	pthread_exit((void *)0);
}

void *fun(void *arg)
{
	for(;;)
	{
		pthread_rwlock_wrlock(&rwlock);
		sleep(1);
		//pthread_cond_wait(&cond, &mutex);
		printf("int fun:globalvbal = %d\n", global_value);
		pthread_rwlock_unlock(&rwlock);
	}
	return (void *)0;
}

void *fun1(void *arg)
{
	for(;;)
	{
		pthread_rwlock_rdlock(&rwlock);
		//pthread_cond_wait(&cond, &mutex);
		printf("int fun1 globalvbal = %d\n", global_value);
		pthread_rwlock_unlock(&rwlock);
	}
	return (void *)0;
}

