#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SIZE_NUM 1024

int main(int argc, char **argv)
{
	ssize_t readlink_num;
	char readlink_buff[SIZE_NUM];

		
       ssize_t readlink(const char *path, char *buf, size_t bufsiz);
	if((readlink_num = readlink(argv[1], readlink_buff, SIZE_NUM)) < 0)
	return 0;
}

