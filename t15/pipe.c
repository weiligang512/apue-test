#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>


void sighandler(int sig)
{
	fprintf(stdout, "signa: %d \n", sig);
}


int main(int argc, char **argv)
{
	int pe[2];

      // typedef void (*sighandler_t)(int);
      // sighandler_t signal(int signum, sighandler_t handler);
	signal(SIGPIPE, SIG_IGN);

       //int pipe(int piped[2]);
	if (pipe(pe) < 0)
	{
		perror("pipe");
		exit(1);
	}
	
	pid_t pid;
	//pid_t fork(void);
	if ((pid = fork()) < 0)
	{
		perror("fork");
		exit(1);
	}
	else if (pid == 0)
	{
		//child
		char buff[128];
		ssize_t num;

		close(pe[0]);
		close(pe[1]);
	//ssize_t read(int fd, void *buf, size_t count);
#if 0
		num = read(pe[0], buff, sizeof(buff));
		if (num == 0)
		{
			fprintf(stdout, "write end closed\n");
			exit(1);
		}
		if (num < 0)
		{
			perror("child read");
			exit(1);
		}
		buff[num] = '\0';

		fprintf(stdout, "[%d]child: read form pipe %d byte : %s\n", getpid(), num, buff);
#endif
		exit(0);
	}
	else
	{
		//parent
		char buff[] = "This pipe test!";
		ssize_t num;
		sleep(2);
		close(pe[0]);
		//ssize_t write(int fd, const void *buf, size_t count);
		if ((num = write(pe[1], buff, sizeof(buff))) < 0)
		{
			perror("parent write");
			close(pe[1]);
			exit(1);
		}

		fprintf(stdout, "[%d]parent: write to pipe %d byte : %s\n", getpid(), num, buff);

		wait(NULL);
	}

	close(pe[0]);
	close(pe[1]);

	return 0;
}
