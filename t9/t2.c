#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <signal.h>


void sighandler(int s)
{
	printf("signa:%d\n", s);
	sleep(10);
}
void sig_alarm(int s)
{
	printf("signa:%d\n", s);
}

int main(int argc, char **argv)
{
#if 0
	// unsigned int alarm(unsigned int seconds);
       int sigemptyset(sigset_t *set);
       int sigfillset(sigset_t *set);
       int sigaddset(sigset_t *set, int signum);
       int sigdelset(sigset_t *set, int signum);
       int sigismember(const sigset_t *set, int signum);
	int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);

	sigset_t s, o;

       	sigemptyset(&s);
	
	sigaddset(&s, SIGABRT);
	
	sigprocmask(SIG_BLOCK, &s, &o);

	if(sigismember(&s, SIGABRT))
		puts("SIGABRT mask already set");
#endif
//	signal(SIGABRT, sig_alarm);
	signal(SIGALRM, sighandler);
	setbuf(stdout, 0);
	fprintf(stdout, "old alarm time:%d\n", alarm(5));
	int sl;
	sl = sleep(20);
	printf("sleep rerurnd:%d", sl);
	int i = 0;
	fprintf(stdout, "old alarm time:%d\n", alarm(5));
	i = pause();
	fprintf(stdout, "pause returned %d\n", i);
	return 0;
}
