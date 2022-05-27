CFLAGS=-std=c11 -g -static
TESTDIR=./tests
SRCDIR=./src
SRCS=$(wildcard $(SRCDIR)/*.c)
TESTSRCS=$(wildcard $(TESTDIR)/*.c)
OBJS=$(SRCS:.c=.o)

kcc: $(OBJS)
		$(CC) -o kcc $(OBJS) $(LDFLAGS)

$(OBJS): $(SRCDIR)/kcc.h

test: kcc
		./tests/test-control.sh

exec: kcc
		./exec.sh

clean:
		find . -name "kcc" -o -name "*.o" -o -name "*~" -o -name "tmp*" | xargs rm -f 

.PHONY: test clean