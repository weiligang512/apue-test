#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

#define LENGTH 1024

int main(int argc, char **argv)
{
	
	//int truncate(const char *path, off_t length);
	//int ftruncate(int fd, off_t length);
	if(truncate(argv[1], LENGTH) < 0)
	{
		fprintf(stderr, "truncate a file %s to specifeid %d failed: %s\n", argv[1], LENGTH, strerror(errno));
		exit(1);
	}
	fprintf(stdout, "truncate a file %s to specifeid %d successfully\n", argv[1], LENGTH);

	return 0;
}
