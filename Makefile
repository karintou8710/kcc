CFLAGS=-std=c11 -g -static
TESTDIR=./tests
SRCDIR=./src
SRCS=$(wildcard $(SRCDIR)/*.c)
TESTSRCS=$(wildcard $(TESTDIR)/*.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
		$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): $(SRCDIR)/9cc.h

test: 9cc test_c
		./tests/test-control.sh

testall: 9cc test_c
		./tests/test-binop.sh && ./tests/test-control.sh

exec: 9cc test_c
		./exec.sh

test_c:
		gcc -xc -c -o tmp2.o $(TESTSRCS)

clean:
		find . -name "9cc" -o -name "*.o" -o -name "*~" -o -name "tmp*" | xargs rm -f 

.PHONY: test clean