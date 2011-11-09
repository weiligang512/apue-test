/* function : mmap practice
 *
 *
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/mman.h>

#define FILE_SIZE 1024

int main(int argc, char **argv)
{
	int fd;
       //int open(const char *pathname, int flags);
       //int open(const char *pathname, int flags, mode_t mode);
	if ((fd = open(argv[1], O_RDONLY)) < 0)
	{
		if (errno == EEXIST)
		{
			fprintf(stderr, "file %s exist %s\n", argv[1], strerror(errno));
			exit(1);
		}
		else
		{
			fprintf(stderr, "open file %s failed %s\n", argv[1], strerror(errno));
			exit(1);
		}
	}
	fprintf(stdout, "open file %s success\n", argv[1]);
	#if 0
       //off_t lseek(int fd, off_t offset, int whence);
	off_t sk;
	if((sk = lseek(fd, FILE_SIZE -1, SEEK_SET)) == -1)
	{
		fprintf(stderr, "seek file %s failed %s\n", argv[1], strerror(errno));
		close(fd);
		exit(1);
	}
	//ssize_t write(int fd, const void *buf, size_t count);
	
	write(fd, "weiligang", 9);
	#endif

	void *map;

        //void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
	//int munmap(void *addr, size_t length);
	//PROT_READ  Pages may be read.
       //PROT_WRITE Pages may be written.
	//MAP_PRIVATE

	if((map = mmap(NULL, FILE_SIZE, PROT_READ, MAP_PRIVATE, fd, SEEK_SET)) == MAP_FAILED)
	{
		fprintf(stderr, "map file to memory failed %s\n", strerror(errno));
		close(fd);
		exit(1);
	}
	time_t time_l;
	char *time_buff;
	while(1)
	{
       // time_t time(time_t *tloc);
	time_l = *(time_t *)(map + 100);
	//ar *ctime(const time_t *timep);	
	time_buff = ctime(&time_l);
	fprintf(stdout, "%s\n", time_buff);
	sleep(1);
	}
	munmap(map, FILE_SIZE);
	
	close(fd);

	return 0;
}
