#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>


typedef void (*sighandler_t)(int);
void sighandler(int sig)
{
	fprintf(stdout, "signal: %d\n", sig);
}

int main(int argc, char **argv)
{
      // sighandler_t signal(int signum, sighandler_t handler);
	sighandler_t old_sighand;
	
	if ((old_sighand = signal(SIGINT, SIG_IGN)) == SIG_ERR)
	{
		perror("signal");
	}

	if ((old_sighand = signal(SIGQUIT, SIG_DFL)) == SIG_ERR)
	{
		perror("signal");
	}

	if ((old_sighand = signal(SIGTERM, sighandler)) == SIG_ERR)
	{
		perror("signal");
	}

	if ((old_sighand = signal(SIGKILL, sighandler)) == SIG_ERR)
	{
		perror("signal");
	}

	if ((old_sighand = signal(SIGSTOP, sighandler)) == SIG_ERR)
	{
		perror("signal");
	}
	
	for (;;)
	{
		fprintf(stdout, ".");
		fflush(stdout);
		
		sleep(1);
	}	

	return 0;
}
