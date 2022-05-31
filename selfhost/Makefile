CFLAGS=-std=c11 -g -static -w
SRCDIR=./src
SRCS=$(wildcard $(SRCDIR)/*.c)
TESTSRCS=$(wildcard $(TESTDIR)/*.c)
OBJS=$(SRCS:.c=.o)

kcc: $(OBJS)
		$(CC) -o kcc $(OBJS) $(LDFLAGS)

$(OBJS): $(SRCDIR)/kcc.h

update:
	cp ../src/*.c ./src

clean:
		find . -name "kcc" -o -name "*.o" -o -name "*~" -o -name "tmp*" | xargs rm -f 

.PHONY: test clean src