#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>


int main(int argc, char **argv)
{
	pid_t pid;
	int i;
	
       //pid_t fork(void);
	if ((pid = fork()) < 0)
	{
		perror("fork");
		exit(1);
	}
	else if (pid == 0)
	{
		//child process
		//pid_t setsid(void);
		if (setsid() == (pid_t)-1)
		{
			perror("setsid");
			exit(1);
		}
	//int chdir(const char *path);
		
		if (chdir("/") < 0)
		{
			perror("chdir");
			exit(1);
		}
       		//mode_t umask(mode_t mask);
		umask(0);
		
		long open_max;
	//	long sysconf(int name);
		open_max = sysconf(_SC_STREAM_MAX);
		for (i = 1; i <= open_max; i++)
		{
			close(i);
		}	
		
		while (1)
		{
			fprintf(stdout, ".");
			sleep(1);
		}
	}
	else
	{
		//parent process
		puts("parent");
		exit(0);
	}
	#if 0
      	 void openlog(const char *ident, int option, int facility);
    	 void syslog(int priority, const char *format, ...);
   	 void closelog(void);
	#endif

	openlog(argv[0], LOG_PID, LOG_USER | LOG_DEBUG);
	syslog(LOG_USER | LOG_DEBUG, "%d start....\n", (int)getpid);
	closelog();	

	return 0;
}
