#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char **argv)
{
//	int system(const char *command);
	int status;

	status = system("ls");
	sleep(2);

	fprintf(stdout, "status = %d\n", status);

	return 0;
}
