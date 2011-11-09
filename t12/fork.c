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
      // pid_t fork(void);
	pid_t pid;

	pid = fork();

	if (pid < 0)
	{
		perror("fork");
	}
	else if(pid == 0)
	{
		sleep(1000);
      		// pid_t getpid(void);
     		 // pid_t getppid(void);
	//	if (fork() == 0)
	//	{
	//		sleep(5);
	//		fprintf(stdout, "childchild: PID = %d, PPID = %d, UID = %d, EUID = %d\n", getpid(), getppid(), getuid(), geteuid());
	//	}
	//	else
	//	{
			fprintf(stdout, "child: PID = %d, PPID = %d, UID = %d, EUID = %d\n", getpid(), getppid(), getuid(), geteuid());
			exit(0);
	//	}
	}
	else
	{
		fprintf(stdout, "parent: PID = %d, PPID = %d, UID = %d, EUID = %d\n", getpid(), getppid(), getuid(), geteuid());
			
		pid_t p;
		int status;

		//pid_t waitpid(pid_t pid, int *status, int options);
		while(!0)
		{
			p = waitpid(-1, &status, WNOHANG);
			fprintf(stdout, "p = %d\n", p);
			if(WIFEXITED(status) == 0)
			{
				puts("no");
			}
			if(WIFEXITED(status) > 0)
			{
				puts("terminated normally");
				fprintf(stdout, "WEXITSTATUS = %08o\n", WEXITSTATUS(status));
				break;
			}
			sleep(1);
		
		}		
			fprintf(stdout, "p = %d\n", p);
		puts("sueccse");	
		exit(0);
	}
	
	//sleep(1000);0

	return 0;
}
