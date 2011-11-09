#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

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
	size_t argv_t = strlen(argv[1]);
	memcpy(un_addr.sun_path, argv[1], argv_t);

	//int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	if (bind(un_fd, (struct sockaddr *)&un_addr, sizeof(un_addr)) < 0)
	{
		perror("bind");
		close(un_fd);
		exit(1);
	}

	struct sockaddr_un peer_addr;
	socklen_t add_len;
	char rec_buff[1024];
	char peer_path[108];
	size_t rec_num;
	for(;;)
	{
		add_len = sizeof(peer_addr);
	//size_t recvfrom(int sockfd, void *buf, size_t len, int flags,
	// struct sockaddr *src_addr, socklen_t *addrlen);
		rec_num = recvfrom(un_fd, rec_buff, sizeof(rec_buff) - 1, 0, (struct sockaddr *)&peer_addr, &add_len);
		if (rec_num < 0)
		{
			fprintf(stderr, "recver from socket failed: %s\n", strerror(errno));

		}
		else
		{
			rec_buff[rec_num] = '\0';
		//	inet_ntop(AF_UNIX, (struct sockaddr *)&peer_addr, peer_path, sizeof(peer_path) - 1);
			//fprintf(stdout, "peer pathname: %s\n", peer_path);
			fprintf(stdout, "peer pathname: %s\n", un_addr.sun_path);
			fprintf(stdout, "recver from socket %d byte: %s\n", rec_num, rec_buff);
		}
	}
#if 0
	// ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
	// const struct sockaddr *dest_addr, socklen_t addrlen);
	send_num = sendto(un_fd, argv[2], sizeof(argv[2]), 0, (struct sockaddr *)&un_addr, sizeof(un_addr));
	if (send_num < 0)
	{
		perror("sendto");
	}
	else
	{
		fprintf(stdout, "send %d byte %s\n", send_num, argv[2]);
	}
#endif

	close(un_fd);
	return 0;
}
