#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>



int main(int argc, char **argv)
{
	int socket_fd;
       //int socket(int domain, int type, int protocol);
	socket_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
	{
		perror("socket");
		exit(1);
	}
	fprintf(stdout, "create socket successs\n");
#if 0
	type = struct sockaddr_in {
		    sa_family_t sin_family;
		        in_port_t sin_port;
			    struct in_addr sin_addr;
			        unsigned char sin_zero[8];
	}
#endif

	struct sockaddr_in addr;
	int port;
	port = strtol(argv[2], NULL, 10);

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	//int inet_pton(int af, const char *src, void *dst);
	inet_pton(AF_INET, argv[1], (struct in_addr *)&addr.sin_addr);
	//int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
	struct sockaddr_in addr1;
	socklen_t len;
	int accept_fd;
	char client[] = "xxx.xxx.xxx.xxx\0";

	listen(socket_fd, 10);
	for(;;)
	{
	len = (socklen_t)sizeof(addr1);
	//int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
	accept_fd = accept(socket_fd, (struct sockaddr *)&addr1, &len);
	if (accept_fd < 0)
	{
		perror("accept");
		break;
	}
	else
	{
		printf("accept success.\n");
		//const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
		inet_ntop(AF_INET, (struct in_addr *)&addr1.sin_addr, client, sizeof(client));
		printf("clinet: IP = %s, port = %d\n", client, ntohs(addr1.sin_port));

	}
	char wbuff[] = "this is server";
	int n;
	n = write(accept_fd, wbuff, sizeof(wbuff));
	printf("read from %s\n", wbuff);
	
	struct hostent *hostent;
	//hostent = gethostbyaddr((struct in_addr *)&addr1.sin_addr, sizeof(struct in_addr), AF_INET);
	hostent = gethostbyname(argv[3]);
	fprintf(stdout, "host name : %s\n", hostent->h_name);
	char **p = NULL;
	p = hostent->h_aliases;
	fprintf(stdout, "alias : ");

	while(p && *p)
	{
		fprintf(stdout, "%s, ", *p);
		p++;
	}
	fprintf(stdout, "\n");

	fprintf(stdout, "IPtype : ");
	if (hostent->h_addrtype == PF_INET)
	{
		fprintf(stdout, "IPv4\n");
	}
	if (hostent->h_addrtype == PF_INET6)
	{
		fprintf(stdout, "IP64\n");
	}

	p = hostent->h_addr_list;
	char IP_buff[] = "xxx.xxx.xxx.xxx\0";

	fprintf(stdout, "IPaddress : ");	
	while(p && *p)
	{
		inet_ntop(AF_INET, *p, IP_buff, sizeof(IP_buff));


		fprintf(stdout, "%s, ", IP_buff);
		p++;
	}
	fprintf(stdout, "\n");

	}


	close(socket_fd);

	return 0;
}
