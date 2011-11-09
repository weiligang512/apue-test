CC	= gcc

CFLAGS	= -Wall -g

LDFLAGS	= -lrt

TARGETS = pipe popen print mq_open mq_send mq_receive sem_open sem sem_trywait sem_fork shm_open

.PHONY: all clean

all :$(TARGETS)

clean:
	rm -f *.o $(TARGETS) *~
