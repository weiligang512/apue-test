#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char **argv)
{
	int sock_fd;

#if 0
	unsigned short port;

//	port = (unsigned short)strtol(argv[2], NULL, 10);
//	fprintf(stdout, "port = %04x\n", port);
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

	struct sockaddr_in {
		    sa_family_t sin_family;
		        in_port_t sin_port;
			    struct in_addr sin_addr;
			        unsigned char sin_zero[8];
	}

	struct hostent {
               char  *h_name;            /* official name of host */
               char **h_aliases;         /* alias list */
               int    h_addrtype;        /* host address type */
               int    h_length;          /* length of address */
               char **h_addr_list;       /* list of addresses */
           }	
#endif 
	struct hostent *hostent;

	//struct hostent *gethostbyname(const char *name);
	//struct hostent *gethostbyaddr(const void *addr,
	//socklen_t len, int type);
	struct in_addr net_addr;

	inet_pton(AF_INET, argv[1], (struct in_addr *)&net_addr);
	//hostent = gethostbyaddr((struct sockaddr *)&local_addr, sizeof(local_addr), AF_INET);
	hostent = gethostbyaddr((struct in_addr *)&net_addr, sizeof(net_addr), AF_INET);
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
	char IP_buff[] = "xxx.xxx.xxx.xxx";

	fprintf(stdout, "IPaddress : ");	
	while(p && *p)
	{
		inet_ntop(AF_INET, *p, IP_buff, sizeof(IP_buff));

		fprintf(stdout, "%s, ", IP_buff);
		p++;
	}
	fprintf(stdout, "\n");

	return 0;
}
