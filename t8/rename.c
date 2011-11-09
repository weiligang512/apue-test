#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


int main(int argc, char **argv)
{
	if(argc != 3)
	{
		fprintf(stderr, "Usage: %s <oldpath> <newpath>\n", argv[0]);
		exit(1);
	}
      // int rename(const char *oldpath, const char *newpath);
	if(rename(argv[1], argv[2]) < 0)
	{
		fprintf(stderr, "change name of file %s failed: %s\n", argv[1], strerror(errno));
		exit(1);
	}
	
	fprintf(stdout, "change name of file %s successfully\n", argv[1]);	

	return 0;
}
