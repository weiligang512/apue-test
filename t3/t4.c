#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int fd;

      // int open(const char *pathname, int flags, mode_t mode);
	
	if((fd = open(argv[1], O_RDWR | O_CREAT | O_EXCL, 0666)) < 0)
	{
		if(errno == EEXIST)
		{
			fprintf(stderr, "file %s is exist %s\n", argv[1], strerror(errno));
			exit(1);
		}
		else
		{
			fprintf(stderr, "open file %s failed %s\n", argv[1], strerror(errno));
			exit(1);
		}
	}
	fprintf(stdout, "open fiel %s success fd = %d\n", argv[1], fd);	
 	
	int fd1;
	
      // int open(const char *pathname, int flags, mode_t mode);
	
	if((fd1 = open(argv[2], O_RDWR | O_CREAT | O_EXCL, 0666)) < 0)
	{
		if(errno == EEXIST)
		{
			fprintf(stderr, "file %s is exist %s\n", argv[2], strerror(errno));
			exit(1);
		}
		else
		{
			fprintf(stderr, "open file %s failed %s\n", argv[2], strerror(errno));
			exit(1);
		}
	}
	fprintf(stdout, "open fiel %s success fd1 = %d\n", argv[2], fd1);	
	
       //int fcntl(int fd, int cmd, ... /* arg */ );
	#if 0
	int get_fd;
	
	if((get_fd = fcntl(fd, F_GETFD)) < 0)
	{
		fprintf(stderr, "fcntl get file descriptor faied %s\n", strerror(errno));
		close(fd);
		exit(1);
	}
	fprintf(stdout, "get file descriptor is get_fd = %d\n", get_fd);
	#endif

	int new_fd;
	
	//int dup(int oldfd);
        //int dup2(int oldfd, int newfd);
	if((new_fd = dup2(fd, fd1)) < 0)
	{
		fprintf(stderr, "duplicate new file descriptor failed %s\n", strerror(errno));
		if(errno == EBADF)
			fprintf(stderr, "old file descriptor is not an open file descriptor %s\n", strerror(errno));

		close(fd);
		exit(1);
	}
	fprintf(stdout, "duplicate new file descroptor success %d\n", new_fd);
	sleep(1000);

	close(fd);
//	close(fd1);
	close(new_fd);
	return 0;
}
