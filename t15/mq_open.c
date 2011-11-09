#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>



int main(int argc, char **argv)
{
	mqd_t mq;
	struct mq_attr attr;

#if 0 
truct mq_attr {
               long mq_flags;       /* Flags: 0 or O_NONBLOCK */
               long mq_maxmsg;      /* Max. # of messages on queue */
               long mq_msgsize;     /* Max. message size (bytes) */
               long mq_curmsgs;     /* # of messages currently in queue */
           };
#endif
	memset(&attr, 0, sizeof(attr));

	attr.mq_maxmsg = 5;
	attr.mq_msgsize = 8192;
	
      // mqd_t mq_open(const char *name, int oflag);
      // mqd_t mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr);
	if((mq = mq_open(argv[1], O_RDWR | O_CREAT | O_EXCL, 0644, &attr)) == (mqd_t)-1)
	{
		perror("mq_open");
		exit(1);
	}
	printf("open or create message queue success\n");	
//	mq_setattr(mq, &attr, NULL);
	
	struct mq_attr get_attr;
      // mqd_t mq_getattr(mqd_t mqdes, struct mq_attr *attr);
      // mqd_t mq_setattr(mqd_t mqdes, struct mq_attr *newattr, struct mq_attr *oldattr);
	mq_getattr(mq, &get_attr);
	fprintf(stdout, "flags = %ld, maxmsg = %ld, msgsize = %ld, curmsgs = %ld\n ", attr.mq_flags, attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs);	

	return 0;
}
