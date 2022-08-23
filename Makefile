CC=gcc
CLANG=clang -fsanitize=address -fno-omit-frame-pointer
AS=as
CFLAGS=-std=c11 -g

SRCS=$(wildcard src/*.c)
HEADERS=$(wildcard src/*.h)
TESTSRCS=$(wildcard tests/*.c)

GEN1_OBJS=$(SRCS:.c=.o)
GEN1_CLANG_OBJS=$(SRCS:%.c=%_clang.o)
GEN2_OBJS=$(GEN1_OBJS:src/%.o=selfhost/gen2/%.o)
GEN3_OBJS=$(GEN1_OBJS:src/%.o=selfhost/gen3/%.o)

GEN1=kcc1
GEN1_CLANG=kcc1_clang
GEN2=kcc2
GEN3=kcc3

TEST_SCRIPT=tests/test-control.sh
TEST_ERROR_SCRIPT=tests/test-error.sh
EXEC_SCRIPT=run.sh
DIFF_SCRIPT=selfhost/diff.sh
TEST_GCC_AND_CLANG=tests/test_gcc_and_clang.sh

# for franken compile
EXCLUDE_FILES=

# for selfhost
DUMMY_LIB_DIR=selfhost/dummy_headers

### first generation ###
$(GEN1): $(GEN1_OBJS)
	$(CC) -o $@ $^

$(GEN1_OBJS): $(HEADERS)

$(GEN1_CLANG): $(GEN1_CLANG_OBJS)
	$(CLANG) -o $@ $^

src/%_clang.o: src/%.c $(HEADERS)
	$(CLANG) -c -o $@ $<

### second generation ###
$(GEN2): $(GEN2_OBJS)
	$(CC) -o $@ $^

selfhost/gen2/%.o: src/%.c $(HEADERS) $(GEN1)
	@mkdir -p $(@D)

# self or gcc
	@if [ -z $(filter $<,$(EXCLUDE_FILES)) ]; then\
		cpp -I $(DUMMY_LIB_DIR) $< > $(@:%.o=%.i);\
		./$(GEN1) $(@:%.o=%.i) > $(@:%.o=%.s);\
		$(CC) -c -o $@ $(@:%.o=%.s);\
	else\
		cpp $< > $(@:%.o=%.i);\
		$(CC) -c -o $@ $(@:%.o=%.i);\
	fi

### third generation ###
$(GEN3): $(GEN3_OBJS)
	$(CC) -o $@ $^

selfhost/gen3/%.o: src/%.c $(HEADERS) $(GEN2)
	@mkdir -p $(@D)
	
# self or gcc
	@if [ -z $(filter $<,$(EXCLUDE_FILES)) ]; then\
		cpp -I $(DUMMY_LIB_DIR) $< > $(@:%.o=%.i);\
		./$(GEN2) $(@:%.o=%.i) > $(@:%.o=%.s);\
		$(CC) -c -o $@ $(@:%.o=%.s);\
	else\
		cpp $< > $(@:%.o=%.i);\
		$(CC) -c -o $@ $(@:%.o=%.i);\
	fi

### tests ###
test1: $(GEN1)
	sh $(TEST_SCRIPT) $(GEN1)

test1_error: $(GEN1)
	sh $(TEST_ERROR_SCRIPT) $(GEN1)

test2: $(GEN2)
	sh $(TEST_SCRIPT) $(GEN2)

test2_error: $(GEN2)
	sh $(TEST_ERROR_SCRIPT) $(GEN2)

test3: $(GEN3)
	sh $(TEST_SCRIPT) $(GEN3)

test3_error: $(GEN3)
	sh $(TEST_ERROR_SCRIPT) $(GEN3)

diff: $(GEN3)
	sh $(DIFF_SCRIPT)

test1_clang: $(GEN1_CLANG)
	sh $(TEST_SCRIPT) $(GEN1_CLANG)

test1_gcc_and_clang: $(GEN1) $(GEN1_CLANG)
	sh $(TEST_GCC_AND_CLANG) $(GEN1) $(GEN1_CLANG)

testall: test1 test1_error test2 test2_error test3 test3_error diff

testextra: test1_clang test1_gcc_and_clang

### run simple file ###
run1: $(GEN1)
	sh $(EXEC_SCRIPT) $(GEN1)

run2: $(GEN2)
	sh $(EXEC_SCRIPT) $(GEN2)

run3: $(GEN3)
	sh $(EXEC_SCRIPT) $(GEN3)

### utils ###
clean:
	find . -name "*.o" -o -name "*~" -o -name "*.i" -o -name "tmp*" -o -path "./selfhost/gen[2-3]/*" | xargs rm -rf
	rm -f kcc1 kcc2 kcc3 kcc1_clang kcc1_gcc a.out out

.PHONY: test1 test1_clang test1_gcc_and_clang test2 test3 diff testall testextra run1 run2 run3 clean test1_error test2_error test3_error