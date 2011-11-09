#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char **argv)
{

	#if 0 
     // pid_t fork(void);
	pid_t pid[3];
	int i;
	
	puts("start");
		fprintf(stdout, "parent: PID = %d, PPID = %d, UID = %d, EUID = %d\n", getpid(), getppid(), getuid(), geteuid());
	
	for(i = 0; i < 3; i++)
	{
		pid[i] = fork();
		printf("pid[i] = %d\n", pid[i]);
	}

	if (pid[0] < 0)
	{
		perror("fork");
	}
	else if(pid[0] == 0)
	{
      		// pid_t getpid(void);
     		 // pid_t getppid(void);
			fprintf(stdout, "child: PID = %d, PPID = %d, UID = %d, EUID = %d\n", getpid(), getppid(), getuid(), geteuid());
		//	exit(0);
	}
	else
	{
		fprintf(stdout, "parent: PID = %d, PPID = %d, UID = %d, EUID = %d\n", getpid(), getppid(), getuid(), geteuid());
			
		pid_t p;

		puts("sueccse");	
		exit(0);
	}
	#endif
	for (;;)

	sleep(1);

	return 0;
}
