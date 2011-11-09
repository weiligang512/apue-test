#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char **argv)
{
      // int access(const char *pathname, int mode);
	fprintf(stdout, "eUID = %d, eGID = %d\n", geteuid(), getegid());
	fprintf(stdout, "UID = %d, GID = %d\n", getuid(), getgid());
	fflush(stdout);
	if(!access(argv[1], W_OK))
	{
		fprintf(stdout, "test file %s is exist\n", argv[1]);	
	}
	else
	{
		fprintf(stderr, "test file %s not exist\n", argv[1]);
	}

	return 0;
}
