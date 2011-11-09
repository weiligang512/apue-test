#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
	char *geten;

       //char *getenv(const char *name);
	geten = getenv("HOME");
	puts(geten);
#if 0
	 // putenv(char *string);
	if(putenv("HOME=bsec") < 0)
	{
		perror("putenv");
		exit(1);
	}
#endif
	//int setenv(const char *name, const char *value, int overwrite);	
	if (setenv("HOME", "bsec", 0) < 0)
	{
		perror("setenv");	
	}
//	puts(geten);
	geten = getenv("HOME");
	puts(geten);
	

	return 0;
}
