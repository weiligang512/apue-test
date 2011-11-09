#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define READ_BUFF 1024

int main(int argc, char **argv)
{
	int socket_fd;
	unsigned short port;
	char rbuff[READ_BUFF + 1];
	int n;
	
	port = strtol(argv[2], NULL, 10);
       //int socket(int domain, int type, int protocol);
	socket_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
	{
		perror("socket");
		exit(1);
	}
	fprintf(stdout, "cteate socket success fd = %d\n", socket_fd);
	struct sockaddr_in addr;
#if 0
	type = struct sockaddr_in {
		    sa_family_t sin_family;
		        in_port_t sin_port;
			    struct in_addr sin_addr;
			        unsigned char sin_zero[8];
	}
#endif
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	//int inet_pton(int af, const char *src, void *dst);
	if (inet_pton(AF_INET, argv[1], (struct in_addr *)&addr.sin_addr) != 1)
	{
		fprintf(stderr, "convert IPv4 to binary failed\n");
		close(socket_fd);
		exit(1);
	}
	//int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	if (connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)	
	{
		perror("connect");
		close(socket_fd);
		exit(1);
	}
	fprintf(stdout, "connect to socket success\n");

//	char buff[] = "GET /\n\n";

//	write(socket_fd, buff, sizeof(buff));

	n = read(socket_fd, rbuff, READ_BUFF);
	rbuff[n] = '\0';

	fprintf(stdout, "%s\n", rbuff);
	close(socket_fd);

	return 0;
}
