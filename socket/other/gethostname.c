#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char **argv)
{
	char hostname[128];

	//int gethostname(char *name, size_t len);
	//int sethostname(const char *name, size_t len);

	memset(hostname, 0, sizeof(hostname));
	gethostname(hostname, sizeof(hostname));

	fprintf(stdout, "hostname is : %s\n", hostname);

	if (sethostname(argv[1], strlen(argv[1])) < 0)
	{
		perror("sethostname");
	}
	memset(hostname, 0, sizeof(hostname));
	gethostname(hostname, sizeof(hostname));

	fprintf(stdout, "hostname is : %s\n", hostname);

	return 0;
}
