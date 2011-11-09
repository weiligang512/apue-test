#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern char **environ;


int main(int argc, char **argv)
{
	int i;
	char **p = environ;

	for(i = 0; i < argc; i++)
	{
		fprintf(stdout, "%s\n", argv[i]);
	}

	while(*p)
	{
		fprintf(stdout, "%s\n", *p++);
	}

	return 0;
}
