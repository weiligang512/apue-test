#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



int main(int argc, char **argv)
{
       //int open(const char *pathname, int flags);
	int fd;
	if((fd = open(argv[1], O_RDWR)) < 0)
	{
		perror("open");
		exit(1);
	}
       //int fcntl(int fd, int cmd, ... /* arg */ );
	int get_fd;
	
	if((get_fd = fcntl(fd, F_GETFD)) < 0)
	{
		perror("fcntl");
		close(fd);
		exit(1);
	}
	fprintf(stdout, "getgd = %d\n", get_fd);
	
	close(fd);

	return 0;
}
