#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	mode_t mode;
	
	if (argc != 3)
	{
		fprintf(stdout, "Usage : %s <pathname> <mode>\n", argv[0]);
		exit(1);
	}

	//long int strtol(const char *nptr, char **endptr, int base)
	mode = (mode_t)strtol(argv[2], NULL, 8);

       //int mkdir(const char *pathname, mode_t mode);
	if (mkdir(argv[1], mode) < 0)
	{
		fprintf(stderr, "create a directory %s failed : %s\n", argv[1], strerror(errno));
		exit(1);
	}
	fprintf(stdout, "create a directory %s successfully\n", argv[1]);

	return 0;
}
