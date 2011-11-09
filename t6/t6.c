#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>



int main(int argc, char **argv)
{
	int fd, fd1;
	char buff[16] = {0};

      // int open(const char *pathname, int flags);
       //int open(const char *pathname, int flags, mode_t mode);
	if ((fd = open(argv[1], O_RDWR | O_CREAT | O_EXCL, 0666)) < 0)
	{
		if (errno == EEXIST)
			perror("open");
		else
			perror("open");
		exit(EXIT_FAILURE);
	}
	if ((fd1 = open(argv[2], O_RDWR | O_CREAT | O_EXCL, 0666)) < 0)
	{
		if (errno == EEXIST)
			perror("open");
		else
			perror("open");
		exit(EXIT_FAILURE);
	}
	//ssize_t write(int fd, const void *buf, size_t count);
	//int dup(int oldfd);
        //int dup2(int oldfd, int newfd);
	#if 0
	int dup_fd1;

	if((dup_fd1 = dup(STDIN_FILENO)) < 0)
	{
		perror("dup");
	}
	else
	{
		fprintf(stdout, "duplicate file %s success %d\n", argv[1], dup_fd1);
	}
	if(read(dup_fd1, buff, sizeof(buff)) < 0)
	{
		perror("write");
	}

	int dup_fd;
	if((dup_fd = dup(STDOUT_FILENO)) < 0)
	{
		perror("dup");
	}	
	else
	{
		fprintf(stdout, "duplicate file %s success %d\n", argv[1], dup_fd);
	}
	if(write(dup_fd, buff, sizeof(buff)) < 0)
	{
		perror("write");
	}
	int dup_fd1;

	if((dup_fd1 = dup2(fd, STDIN_FILENO)) < 0)
	{
		perror("dup");
	}
	else
	{
		fprintf(stdout, "%s %d: uplicate file %s success %d\n",__FILE__, __LINE__, argv[1], dup_fd1);
	}
	if(read(dup_fd1, buff, sizeof(buff)) < 0)
	{
		perror("write");
	}

	int dup_fd;
	if((dup_fd = dup2(fd, STDOUT_FILENO)) < 0)
	{
		perror("dup");
	}	
	else
	{
		fprintf(stdout, "duplicate file %s success %d\n", argv[1], dup_fd);
	}
	if(write(dup_fd, buff, sizeof(buff)) < 0)
	{
		perror("write");
	}
	if((dup_fd = dup2(dup_fd, STDOUT_FILENO)) < 0)
	{
		perror("dup");
	}	
	if((dup_fd1 = dup2(dup_fd1, STDIN_FILENO)) < 0)
	{
		perror("dup");
	}	
	puts("success");

	#endif

	//int dup2(int oldfd, int newfd);
	dup2(STDOUT_FILENO, fd);
	dup2(fd1, STDOUT_FILENO);
	dup2(fd, STDOUT_FILENO);
	
	write(fd, "write", 5);
	printf("%d, %d\n", fd, dup(STDOUT_FILENO));
	printf("%d, %d\n", STDIN_FILENO, STDOUT_FILENO);
//	sleep(1000);	
	close(fd);
//	close(dup_fd);
//	close(dup_fd1);

	return 0;
}
