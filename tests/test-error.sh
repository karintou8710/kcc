#!/bin/bash

COMPILER=$1
DUMMY_LIB_DIR=selfhost/dummy_headers
SUCCESS=0
FAILURE=1

debug() {
    FILENAME=`basename "$0"`
    echo "$FILENAME: $1"
}

if [ ! -e "$COMPILER" ]; then
    debug "./$COMPILER does not exist"
    exit $FAILURE
fi

run() {
    input=$1
    src="tmp.c"
    echo "$input" > $src

    debug "$COMPILER start compileing \"$input\""
    pre=`echo $src | sed -e 's/\.c/\.i/g'`
    cpp -w $src > $pre
    ASAN_OPTIONS=detect_leaks=0 ./$COMPILER $pre >/dev/null 2>&1
    ERRCHK=$?
    if [ $ERRCHK -ne $FAILURE ]; then
        debug "error: $COMPILER passed invalid code"
        exit $FAILURE
    fi
}

run "int main() {return 0}"
run "int main() {int a;int a;}"
run "int main() {struct A {}; struct A{};}"
run "int main() {a int = 1;}"
run "int main() {\";}"
run "int f(int a, int a) {} int main() {}"
run "int a(int); int a(int, int); int main() {}"
run "int a() {} int a() {} int main() {}"
run "int a(int) {} int main() {}"
run "int main() { case 1: return 0; }"
run "int main() { default: return 0; }"
run "int main() { (int[20])(1); }"
run "int main() { int a = 1; a.a = 20; }"
run "int main() { struct A { } a; a.a = 1; }"
run "int main() { int a; a(); }"

# const
run "int main() {const int a = 1; a = 2;}"
run "int main() {int *const a = 1; a =2;}"
run "int main() {int a = 10; const int *b = &b; *a = 10; }"
run "typedef const int A; int main() {A a = 10; a = 29;}"
run "typedef int *A; int main() {const A a; a = 1;}"
run "int main() {const int a[3]; a[0] = 1;}"
run "int main() {const int a[3][3]; a[0][0] = 1;}"
run "typedef int A[3][3]; int main() {const A a; a[0][0] = 1;}"
run "int main() {const int a[1];*a = 10;}"
# TODO: memberのconst
run "int main() {const struct A{int member;} a, b; b = a;}"
run "int main() {const union A{int member;} a, b; b = a;}"

# storage
run "typedef int a = 1; int main() {}"
run "extern int a = 1; int main() {}"

# namespace
run "int test;int test(){} int main() {return 0;}"
run "int test(){} int test; int main() {return 0;}"

# func pointer
run "int f() {}int main() {long *a = f;int res = a();return 0;}"



echo "test-error.sh success!"