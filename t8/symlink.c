#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char **argv)
{
	if(argc != 3)
	{
		fprintf(stdout, "Usage: %s <oldpath> <newpath>\n", argv[0]);
		exit(1);
	}
       //int symlink(const char *oldpath, const char *newpath);
	if(symlink(argv[1], argv[2]) < 0)
	{
		fprintf(stderr, "make a new name for file %s failed %s\n", argv[1], strerror(errno));
		exit(1);
	}	
	fprintf(stdout, "make a new name for file %s successfully\n", argv[1]);

	return 0;
}
