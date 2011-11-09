CC := gcc

CFLAGS := -Wall -g

TARGETS := fcntl exit signal test sigaction sigsuspend t2 system exec\
	   shift

.PHONY: all clean

all: $(TARGETS)

clean:
	rm -f *.o *~ 1.file $(TARGETS)
