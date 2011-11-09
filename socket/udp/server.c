#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, char **argv)
{
	int socket_fd;
	unsigned char port;

	port = strtol(argc[2], NULL, 10);
	socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0)
	{
		perror("socket");
		exit(1);
	}
	fprintf(stdout, "create socket success\n")

	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htos(port);
	inet_pton(AF_INET, argv[1], (struct in_addr *)&addr.sin_addr);

	if (bind(socket_fd, (struct socket *)&addr, sizeof(addr)) < 0)
	{
		perror("bind");
		close(socket_fd);
		exit(1);
	}
	fprintf(stdout, "bind a name to socket success\n");

	struct socket_in peer_addr;
	ssize_t rec_num;
	char rec_buff[1024];
	socklen_t socklen;
	char peer_ip[] = "xxx.xxx.xxx.xxx\0";

	//ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
	//struct sockaddr *src_addr, socklen_t *addrlen);
	recvfrom(socket_fd, rec_buff, sizeof(rec_buff) - 1, 0, (struct sockaddr *)&peer_addr, sizeof(peer_addr));

	return 0;
}
