#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>


int main(int argc, char **argv)
{
	#if 0
       mqd_t mq_open(const char *name, int oflag);
       mqd_t mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr);
       mqd_t mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio);
	ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio);
	#endif
	mqd_t mq;
	if ((mq = mq_open(argv[1], O_RDONLY)) == (mqd_t)-1)
	{
		perror("mq_open");
		exit(1);
	}	
	fprintf(stdout, "open message queue success\n");

	unsigned int priority;
	char buff[8192];
	
	priority = strtol(argv[2], NULL, 10);

	if (mq_receive(mq, buff, sizeof(buff), &priority) < 0)
	{
		perror("mq_receive");
		mq_close(mq);
		exit(1);
	}
	fprintf(stdout, "receive a message to a message queue success\n");		

	puts(buff);

	mq_close(mq);

	return 0;
}
