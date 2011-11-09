#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, char **argv)
{
	int fd;

       //int dup(int oldfd);
       //int dup2(int oldfd, int newfd);

	if ((fd = open(argv[1], O_WRONLY | O_CREAT, 0666)) < 0)
	{
		fprintf(stderr, "open file %s failed %s\n", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}
	fprintf(stdout, "open file %s success\n", argv[1]);

	int dup_fd;
	int std_save;

	std_save = dup(STDOUT_FILENO);
	if((dup_fd = dup2(fd, STDOUT_FILENO)) < 0)
	{
		fprintf(stderr, "duplicate file descriptor failed %s\n", strerror(errno));
		close(fd);
		exit(EXIT_FAILURE);
	}

	puts("weiligang");
	dup2(std_save, STDOUT_FILENO);
	puts("12345678");

	close(fd);
	close(dup_fd);
	close(std_save);
	return 0;
}
