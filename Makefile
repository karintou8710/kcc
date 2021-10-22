CFLAGS=-std=c11 -g -static
TESTDIR=./tests
SRCS=$(wildcard *.c)
TESTSRCS=$(wildcard $(TESTDIR)/*.c)
OBJS=$(SRCS:.c=.o)
ARG=main() {p();}

9cc: $(OBJS)
		$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc test_c
		./tests/test-control.sh

testall: 9cc test_c
		./tests/test-binop.sh && ./tests/test-control.sh

exec: 9cc test_c
		./exec.sh "$(ARG)"

test_c:
		gcc -xc -c -o tmp2.o $(TESTSRCS)

clean:
		rm -f 9cc *.o *~ tmp*

.PHONY: test clean