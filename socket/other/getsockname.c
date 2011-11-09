#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


int main(int argc, char **argv)
{
	int sock_fd;
	unsigned short port;

	port = (unsigned short)strtol(argv[2], NULL, 10);
	fprintf(stdout, "port = %04x\n", port);
	sock_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0)
	{
		perror("socket");
		exit(1);
	}
	fprintf(stdout, "create new socket success\n");
	struct sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(port);
	inet_pton(AF_INET, argv[1], (struct in_addr *)&local_addr.sin_addr);
	//int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	if (bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0)
	{
		perror("bind");
		close(sock_fd);
		exit(1);
	}

	struct sockaddr_in addr;
	socklen_t addr_len;
	char IPv4_addr[] = "xxx.xxx.xxx.xxx\0";

	addr_len = sizeof(addr);
	fprintf(stdout, "addr_len = %d\n", addr_len);

#if 0
	struct sockaddr_in {
		    sa_family_t sin_family;
		        in_port_t sin_port;
			    struct in_addr sin_addr;
			        unsigned char sin_zero[8];
	}
#endif 
	//int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
	if (getsockname(sock_fd, (struct sockaddr *)&addr, &addr_len) < 0)
	{
		perror("getsockname");
	}
	else
	{
		fprintf(stdout, "net port = %04xu\n", addr.sin_port);
		inet_ntop(AF_INET, (struct in_addr *)&addr.sin_addr, IPv4_addr, sizeof(IPv4_addr));
		fprintf(stdout, "part = %hu, IP = %s\n", ntohs(addr.sin_port), IPv4_addr);		
	}

	close(sock_fd);

	return 0;
}
