CC=gcc
AS=as
CFLAGS=-std=c11 -g -static

SRCS=$(wildcard src/*.c)
HEADERS=$(wildcard src/*.h)
TESTSRCS=$(wildcard tests/*.c)

OBJS=$(SRCS:.c=.o)
GEN2_OBJS=$(OBJS:src/%.o=selfhost/gen2/%.o)

DUMMY_LIB_DIR=selfhost/dummy_headers

GEN1=kcc
GEN2=kcc2
GEN3=kcc3

TEST_SCRIPT=tests/test-control.sh
EXEC_SCRIPT=exec.sh



# first generation
$(GEN1): $(OBJS)
		$(CC) -o $@ $^ $(LDFLAGS)

$(OBJS): $(HEADERS)

# second generation
$(GEN2): $(GEN2_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

selfhost/gen2/%.o: src/%.c $(HEADERS) $(GEN1)
	@mkdir -p $(@D)
	echo $(@:%.o=%.i)
	cpp -I $(DUMMY_LIB_DIR) $< > $(@:%.o=%.i)
	./$(GEN1) $(@:%.o=%.i) > $(@:%.o=%.s)
	$(AS) -o $@ $(@:%.o=%.s)

# third generation

# tests

test: $(GEN1)
		sh $(TEST_SCRIPT)

exec: $(GEN1)
		sh $(EXEC_SCRIPT)

clean:
		find . -name "kcc" -o -name "kcc2" -o -name "*.o" -o -name "*~" -o -name "tmp*" | xargs rm -f 

.PHONY: test clean