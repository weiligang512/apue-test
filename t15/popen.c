#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int main(int argc, char **argv)
{
	char buff[128];

       //FILE *popen(const char *command, const char *type);
	//int pclose(FILE *stream);
	FILE *fp;
	if ((fp = popen("./print", "r")) == NULL)
	{
		perror("popen");
		exit(1);
	}
	fscanf(fp, "%s", buff);

	fprintf(stdout, "%s", buff);
	pclose(fp);

	return 0;
}
