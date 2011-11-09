#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>



void sighandler(int sig)
{
	fprintf(stdout, "signal: %d\n", sig);
}

int main(int argc, char **argv)
{
#if 0
struct sigaction {
               void     (*sa_handler)(int);
               void     (*sa_sigaction)(int, siginfo_t *, void *);
               sigset_t   sa_mask;
               int        sa_flags;
               void     (*sa_restorer)(void);
           };
#endif

	struct sigaction act, oldact;

	act.sa_handler = sighandler;

	sigset_t sig;
	//int sigemptyset(sigset_t *set);
	sigemptyset(&sig);

       //int sigaddset(sigset_t *set, int signum);
	act.sa_flags = 0;
	//int sigaction(int signum, const struct sigaction *act,
	//struct sigaction *oldact);

	sigaction(SIGINT, &act, &oldact);
	sigaction(SIGQUIT, &act, &oldact);
	sigaction(SIGKILL, &act, &oldact);

	for (;;)
	{
		fprintf(stdout, ".");
		fflush(stdout);
		
		sleep(1);
	}	

	return 0;
}
