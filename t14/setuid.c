#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>



int printf_id(void)
{
	//uid_t getuid(void);
       	//uid_t geteuid(void);

	fprintf(stdout, "uid = %d, gid = %d\n", (int)getuid(), (int)getgid());
	fprintf(stdout, "euid = %d, egid = %d\n", (int)geteuid(), (int)getegid());

	return 0;
}


int main(int argc, char **argv)
{
	printf_id();

      // int setuid(uid_t uid);
	if(setreuid(1000, 1002) < 0)
	{
		perror("setuid");
	}

	if(setregid(1000, 1002) < 0)
	{
		perror("setgid");
	}

	printf_id();

	return 1;
}
