#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>



int main(int argc, char **argv)
{
#if 0
       sem_t *sem_open(const char *name, int oflag);
       sem_t *sem_open(const char *name, int oflag, mode_t mode, unsigned int value);

#endif
	if (argc != 4)
	{
		fprintf(stderr, "Usage: %s <semaphorename> <mode> <initvalue>\n", argv[0]);
		exit(1);
	}
	unsigned int value;
	mode_t mode;
	
	value = strtol(argv[3], NULL, 10);
	mode = (mode_t)strtol(argv[2], NULL, 8);
	sem_t *sem;	

	if ((sem = sem_open(argv[1], O_RDWR | O_CREAT | O_EXCL, mode, value)) == SEM_FAILED)
	{
		perror("sem_open");
		exit(1);
	}
	fprintf(stdout, "create a named semaphor successfully\n");

	return 0;
}
