#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>



int main(int argc, char *argv[])
{
      // int chmod(const char *path, mode_t mode);
      // int fchmod(int fd, mode_t mode);
      //long int strtol(const char *nptr, char **endptr, int base);
	fprintf(stdout, "eUID = %d, eGID = %d\n", geteuid(), getegid());
        fprintf(stdout, "UID = %d, GID = %d\n", getuid(), getgid());

	if (argc != 3)
	{
		fprintf(stdout, "Usage: %s <mode> <filename>\n", argv[0]);
		exit(1);
	}
	mode_t md;
	
	md =  (mode_t)strtol(argv[1], NULL, 8);

	if (chmod(argv[2], md) < 0)
	{
		if (errno == ENOENT)
		{
			fprintf(stderr, "file %s not exist: %s\n", argv[2], strerror(errno));
		}
		else
		{
			fprintf(stderr, "change permissions of a file %s failed: %s\n", argv[2], strerror(errno));
		}
	}
	else
	{
		fprintf(stdout, "change permissions of a file %s successed\n", argv[2]);
	}

	return 0;
}
