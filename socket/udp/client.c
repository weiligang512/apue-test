#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>


int main(int argc, char **argv)
{
	int socket_fd;
	unsigned short port;

	port = strtol(argv[2], NULL, 10);
       //int socket(int domain, int type, int protocol);
	socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0)
	{
		perror("socket");
		exit(1);
	}
	fprintf(stdout, "create a socket success\n");
	struct sockaddr_in addr;
	size_t send_num;
#if 0
	truct sockaddr_in {
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
	inet_pton(AF_INET, argv[1], (struct in_addr *)&addr.sin_addr);
	//ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
	//const struct sockaddr *dest_addr, socklen_t addrlen);
	send_num = sendto(socket_fd, argv[3], strlen(argv[3]), 0, (struct sockaddr *)&addr, sizeof(addr));
	if (send_num < 0)
	{
		fprintf(stderr, "send to socket failed\n");
	}
	else
	{
		fprintf(stdout, "send to socket %d byte %s\n", send_num, argv[3]);
	}
	struct sockaddr_in addr1;
	socklen_t addr1_len;
	char rec_buff[1024];
	size_t rec_num;
	char src_ip[] = "xxx.xxx.xxx.xxx\0";
	memset(&addr1, 0, sizeof(addr1));
	addr1_len = (socklen_t)sizeof(addr1);

	//ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
	//struct sockaddr *src_addr, socklen_t *addrlen);
	rec_num = recvfrom(socket_fd, rec_buff, sizeof(rec_buff) - 1, 0, (struct sockaddr *)&addr1, &addr1_len);
	rec_buff[rec_num] = '\0';
	memset(src_ip, 0, sizeof(src_ip));
	//const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
	inet_ntop(AF_INET, &addr1.sin_addr, rec_buff, sizeof(rec_buff));

	if (rec_num < 0)
	{
		fprintf(stderr, "recver from socket failed\n");

	}
	else
	{
		fprintf(stdout, "recver from IP = %s :", src_ip);
		fprintf(stdout, "%d byte %s\n", rec_num, rec_buff);
	}

	close(socket_fd);

	return 0;
}
