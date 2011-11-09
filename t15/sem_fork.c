#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>		/* For O_* constants */
#include <sys/stat.h>		/* For mode constants */
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int
getvalue (sem_t * sem)
{
  int sval;

  if (sem_getvalue (sem, &sval) < 0)
    {
      perror ("sem_getvalue");
      sem_close (sem);
      exit (1);
    }
  fprintf (stdout, "get semaphore value successfully: value = %d\n", sval);
      fflush(stdout);

  return 0;
}

int
main (int argc, char **argv)
{
#if 0
  sem_t *sem_open (const char *name, int oflag);
  sem_t *sem_open (const char *name, int oflag, mode_t mode,
		   unsigned int value);
  int sem_getvalue (sem_t * sem, int *sval);
  int sem_wait (sem_t * sem);
  int sem_post (sem_t * sem);

#endif
  if (argc != 2)
    {
      fprintf (stderr, "Usage: %s <semaphorename>\n", argv[0]);
      exit (1);
    }
  sem_t *sem;

  if ((sem = sem_open (argv[1], O_RDWR)) == SEM_FAILED)
    {
      perror ("sem_open");
      exit (1);
    }
  fprintf (stdout, "open a named semaphore successfully\n");

  pid_t pid;

  pid = fork ();
  if (pid < 0)
    {
      perror ("fork");
      sem_close (sem);
      exit (1);
    }
  else if (pid == 0)
    {
      puts ("child");
  getvalue (sem);

      if (sem_wait (sem) < 0)
	{
	  perror ("sem_wait");
	}
      else
	{
	  fprintf (stdout, "lock a seamphore successfully\n");
	}

      getvalue (sem);
      puts ("child can I go to here ?");
      fflush(stdout);

      if (sem_post (sem) < 0)
	{
	  perror ("sem_post");
	}
      else
	{
	  fprintf (stdout, "unlock a semaphore successfully\n");
	}
      getvalue (sem);
      fflush(stdout);
      exit(0);

    }
  else
    {
      puts ("parent");
  getvalue (sem);
      if (sem_wait (sem) < 0)
	{
	  perror ("sem_wait");
	}
      else
	{
	  fprintf (stdout, "lock a seamphore successfully\n");
	}

      getvalue (sem);
      puts ("parent can I go to here ?");
      fflush(stdout);
	sleep(5);
      if (sem_post (sem) < 0)
	{
	  perror ("sem_post");
	}
      else
	{
	  fprintf (stdout, "unlock a semaphore successfully\n");
	}
      getvalue (sem);
      fflush(stdout);
      wait(NULL);
    }
  sem_close (sem);

  return 0;
}
