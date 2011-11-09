#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

void signal_handler (int s);

int
main (int argc, char **argv)
{
  signal (SIGALRM, signal_handler);

  sigset_t s, o;
  //int sigemptyset(sigset_t *set);
  sigemptyset (&s);
#if 0
  //int sigaddset(sigset_t *set, int signum);
  sigaddset (&s, SIGALRM);
  //int sigprocmask(int how, const sigset_t * set, sigset_t * oldset);
  if (sigprocmask (SIG_BLOCK, &s, &o) < 0)
    {
      fprintf (stderr, "Change signal mask failed: %s\n", strerror (errno));
      exit (1);
    }
#endif
   alarm (5);
   sleep (10);
 //  sigpending(&s);
 //  if(sigismember(&s, SIGALRM))
 //  {
 //  	puts("SIGALRM");
  // }

  //int sigdelset(sigset_t *set, int signum);
//  sigdelset (&o, SIGALRM);

  //int sigsuspend(const sigset_t * mask);
  int val = 0;
if (val == 1)
{
	pause();
}
 // val = sigsuspend (&o);
  printf ("val = %d\n", val);
val = 1;
  fprintf (stdout, "Can I go here?\n");

  return 0;
}

void
signal_handler (int s)
{
  fprintf (stdout, "Caught signal %d.\n", s);
//  raise (SIGALRM);
}
