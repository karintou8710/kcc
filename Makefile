GCC=gcc
CLANG=clang -fsanitize=address -fno-omit-frame-pointer
AS=as
CFLAGS=-std=c11 -g -static

SRCS=$(wildcard src/*.c)
HEADERS=$(wildcard src/*.h)
TESTSRCS=$(wildcard tests/*.c)

GEN1_OBJS=$(SRCS:.c=.o)

GEN1_GCC=kcc1_gcc
GEN1_CLANG=kcc1_clang

TEST_SCRIPT=tests/test-control.sh
TEST_GCC_AND_CLANG=tests/test_gcc_and_clang.sh

# for franken compile
EXCLUDE_FILES=

# for selfhost
DUMMY_LIB_DIR=selfhost/dummy_headers

### first generation ###
$(GEN1_GCC): $(GEN1_OBJS)
	$(GCC) -o $@ $^

$(GEN1_CLANG): $(GEN1_OBJS)
	$(CLANG) -o $@ $^

$(GEN1_GCC_OBJS): $(HEADERS)

$(GEN1_CLANG_OBJS): $(HEADERS)

### tests ###
test1_gcc: $(GEN1_GCC)
	sh $(TEST_SCRIPT) $(GEN1_GCC)

test1_clang: $(GEN1_CLANG)
	sh $(TEST_SCRIPT) $(GEN1_CLANG)

test1_gcc_and_clang: $(GEN1_GCC) $(GEN1_CLANG)
	sh $(TEST_GCC_AND_CLANG) $(GEN1_GCC) $(GEN1_CLANG)

### utils ###
clean:
	find . -name "kcc[1-3]" -o -name "*.o" -o -name "*~" -o -name "tmp*" -o -path "./selfhost/gen[2-3]/*" | xargs rm -f

.PHONY: test1_gcc test1_clang clean