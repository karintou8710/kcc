CFLAGS=-std=c11 -g -static
TESTDIR=./tests
SRCDIR=./src
SRCS=$(wildcard $(SRCDIR)/*.c)
TESTSRCS=$(wildcard $(TESTDIR)/*.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
		$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): $(SRCDIR)/9cc.h

test: 9cc
		./tests/test-control.sh

exec: 9cc
		./exec.sh

clean:
		find . -name "9cc" -o -name "*.o" -o -name "*~" -o -name "tmp*" | xargs rm -f 

.PHONY: test clean