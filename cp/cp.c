#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#define TRUE !0
#define FLASE 0

int main(int argc, char **argv)
{
	int old_fd, new_fd;
	char buff[128];
	int read_byts;
	int flag = FLASE;

	if (argc != 3)
	{
		fprintf(stdout, "Usage: %s <old filename> <new filename>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
       //int open(const char *pathname, int flags);

	if ((old_fd = open(argv[1], O_RDONLY)) < 0)
	{
		fprintf(stderr, "Opne file %s failed: %s\n", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}
	
       //int open(const char *pathname, int flags, mode_t mode);
	if ((new_fd = open(argv[2], O_WRONLY | O_CREAT | O_EXCL, 0666)) < 0)
	{
		if (errno == EEXIST)
		{
			fprintf(stderr, "file %s exist: %s\n", argv[2], strerror(errno));
		}
		else
		{
			fprintf(stderr, "open file %s failed: %s\n", argv[2], strerror(errno));
		}
		close(old_fd);
		exit(EXIT_FAILURE);
	}
	
      // ssize_t read(int fd, void *buf, size_t count);
	while (!flag)
	{
		if ((read_byts = read(old_fd, buff, sizeof(buff))) < 0)
		{
			fprintf(stderr, "reading file %s failed: %s\n", argv[1], strerror(errno));
		}
		else
		{
			if (read_byts != sizeof(buff))
				flag = TRUE;
		}

		if (write(new_fd, buff, read_byts) < 0)
		{
			fprintf(stderr, "writing file %s failed: %s\n", argv[2], strerror(errno));
		}
	}
	
	close(old_fd);
	close(new_fd);

	return 0;
}
