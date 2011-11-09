#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


int main(int argc, char **argv)
{
       //int remove(const char *pathname);
	if (remove(argv[1]) < 0)
	{
		fprintf(stderr, "remove a file %s name failed: %s\n", argv[1], strerror(errno));
		exit(1);
	}

	fprintf(stdout, "remove a file %s name successfully\n", argv[1]);

	return 0;
}
