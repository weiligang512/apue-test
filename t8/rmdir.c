#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char **argv)
{
      // int rmdir(const char *pathname);
	if(rmdir(argv[1]) < 0)
	{
		fprintf(stderr, "delete a dirctory %s failed : %s\n", argv[1], strerror(errno));
		exit(1);
	}
	fprintf(stdout, "delete a dirctory %s successfully\n", argv[1]);
	
	return 0;
}
