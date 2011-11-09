#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int un_fd;

	un_fd = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (un_fd < 0)
	{
		perror("socket");
		exit(1);
	}
	fprintf(stdout, "create a socket success\n");
#if 0
	struct sockaddr_un {
		    sa_family_t sun_family;
		        char sun_path[108];
	}
#endif

	struct sockaddr_un un_addr;
	un_addr.sun_family = AF_UNIX;
	ssize_t send_num;
	char send_buff[] = "hello";
	size_t sbuff_t = strlen(send_buff);
	size_t argv_t = strlen(argv[1]);
	memcpy(un_addr.sun_path, argv[1], argv_t);
	// ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
	// const struct sockaddr *dest_addr, socklen_t addrlen);
	send_num = sendto(un_fd, send_buff, sbuff_t, 0, (struct sockaddr *)&un_addr, sizeof(un_addr));
	if (send_num < 0)
	{
		perror("sendto");
	}
	else
	{
		fprintf(stdout, "send %d byte: %s\n", send_num, send_buff);
	}

	close(un_fd);
	return 0;
}
