#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>



int main(int argc, char **argv)
{
       //int open(const char *pathname, int flags);

	int fd;
	pid_t pid;
	//pid_t fork(void);
	if ((pid = fork()) < 0)
	{
		perror("fork");
	}
	else if(pid == 0)
	{
	if((fd = open("./1.file", O_WRONLY | O_TRUNC)) < 0)
	{
		perror("open");
		exit(1);
	}
       //ssize_t write(int fd, const void *buf, size_t count);
		if(write(fd, "child", 5) < 5)
		{
			perror("write");
		}
		_exit(0);
	}
	else
	{

		sleep(1);
		if(write(fd, "parent", 6) < 5)
		{
			perror("write");
		}
		wait(NULL);
	}

	return 0;
}
