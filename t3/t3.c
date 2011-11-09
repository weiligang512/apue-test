#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


main (int argc, char **argv)
{
  int fd;

#if 0
  S_IRWXU 00700 user (file owner) has read, write and execute permission
    S_IRUSR 00400 user has read permission
    S_IWUSR 00200 user has write permission
    S_IXUSR 00100 user has execute permission
    S_IRWXG 00070 group has read, write and execute permission
    S_IRGRP 00040 group has read permission
    S_IWGRP 00020 group has write permission
    S_IXGRP 00010 group has execute permission
    S_IRWXO 00007 others have read, write and execute permission
    S_IROTH 00004 others have read permission
    S_IWOTH 00002 others have write permission
    S_IXOTH 00001 others have execute permission
#endif
  if ((fd = open (argv[1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) < 0)
  {
	if (errno == EEXIST)
	{
		fprintf(stderr, "file exist %s\n", strerror(errno));
	}
	else
	{
		fprintf(stderr, "open file %s failed %s\n", argv[1], strerror(errno));
		exit(1);
	}
  }


	int newfd;

	#if 0	
       int fcntl(int fd, int cmd, ... /* arg */ );
		F_DUPFD (long)
              Find  the  lowest  numbered  available  file  descriptor  greater than or equal to arg and make it be a copy of fd.  This is different from
              dup2(2), which uses exactly the descriptor specified.

              On success, the new descriptor is returned.

		Duplicating descriptor
              See dup(2) for further details.
	#endif
	if((newfd = fcntl(fd, F_DUPFD, 10)) < 0)
	{
		fprintf(stderr, "Duplicating a file descriptor failed %s\n", strerror(errno));
	}
	fprintf(stdout, "%d\n", newfd);

	//off_t lseek(int fd, off_t offset, int whence);
	#if 0
	off_t ls;
	
	if((ls = lseek(fd, 8, SEEK_SET)) == -1)
	{
		fprintf(stderr, "offset file %s failed %s", argv[1], strerror(errno));
		exit(1);
	}
	#endif
	//size_t read(int fd, void *buf, size_t count);
	size_t wr_num;
	char wr_buff[] = "writebuff";
	if((wr_num  = write(fd, wr_buff, sizeof(wr_buff))) < 0)
	{
		fprintf(stderr, "%s: %d: write file %s failed %s\n", __FILE__, __LINE__, argv[1], strerror(errno));
	}


	size_t read_num;
	char buff[1024];
	if((read_num = read(newfd, buff, sizeof(buff))) < 0)
	{
		fprintf(stderr, "read file %s failed %s\n", argv[1], strerror(errno));
		exit(1);
	}

	fprintf(stdout, "%s\n", buff);
	close(fd);
	close(newfd);
    return 0;
}
