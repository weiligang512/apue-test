#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <errno.h>
#include <unistd.h>

int main(int argc, char **argv)
{
#if 0
       int shm_open(const char *name, int oflag, mode_t mode);
       int shm_unlink(const char *name);

#endif
       int mode;
       mode = strtol(argv[2], NULL, 8);

	int shm_fd;
		
	shm_fd = shm_open(argv[1], O_RDWR | O_CREAT | O_EXCL, mode);
	if (shm_fd < 0)
	{
		fprintf(stdout, "create a shared memory objects %s failed: %s\n", argv[1], strerror(errno));

		exit(1);
	}
	fprintf(stdout, "shm_fd = %d\n", shm_fd);
	sleep(1000);

	return 0;
}
