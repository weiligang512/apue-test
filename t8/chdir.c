#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define SIZE_NUM 1024

void print_cwd(void)
{
      // char *getcwd(char *buf, size_t size);
	char buff[SIZE_NUM];
	
	if(!(getcwd(buff, SIZE_NUM)))
	{
		
	}
	else
	{
		fprintf(stdout, "CWD:%s\n", buff);
	}
}
int main(int argc, char **argv)
{
	print_cwd();

       //int chdir(const char *path);
	if(chdir(argv[1]) < 0)
	{
		fprintf(stderr, "change working directory to %s failed : %s\n", argv[1], strerror(errno));
		exit(1);
	}
	fprintf(stdout, "change working directory to %s successfully\n", argv[1]); 

	print_cwd();	
	return 0;
}
