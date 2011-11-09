#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

extern char **environ;
int main(int argc, char **argv)
{
//	int system(const char *command);
	int status;
//	int execv(const char *path, char *const argv[]);
	char *av[] = {"ls", "-l", NULL};
	status =  execvp("ls", av);
	sleep(2);

	fprintf(stdout, "status = %d\n", status);

	return 0;
}
