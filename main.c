#include "9cc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません。");
        return EXIT_FAILURE;
    }

    user_input = argv[1];
    label_if_count = 0, label_loop_count = 0;

    token = tokenize(user_input);
    program();
    codegen();

    return EXIT_SUCCESS;
}