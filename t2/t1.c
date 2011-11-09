#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[])
{
	int fd;
	int write_num;
	off_t seek_num;
	char buff[1024];


	if ((fd = open(argv[1], O_WRONLY)) < 0)
	{ 	//char *strerror(int errnum);
		fprintf(stderr, "open %s failed in man %s\n", argv[1], strerror(errno));
		exit(1);
	}
//off_t lseek(int fd, off_t offset, int whence);
	if((seek_num = lseek(fd, 4096, SEEK_END)) == -1)
	{
		fprintf(stderr, "lseek file %s failed %s\n", argv[1], strerror(errno));
		exit(0);
	}
	//ssize_t write(int fd, const void *buf, size_t count);
	if((write_num = write(fd, "w", 1)) < 0)
	{
		fprintf(stderr, "write file %s failed %s\n", argv[1], strerror(errno));
		exit(0);
	}
	int fd2;
	fd2 = dup2(fd, STDIN_FILENO);
	fscanf(stdin, "%s", buff);
	fprintf(stdout, "%s, wieli %d\n", __FILE__, __LINE__);
	printf("%d\n", fd2);
	close(fd);

	return 0;
}
