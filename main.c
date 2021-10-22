#include "9cc.h"

void init() {
    label_if_count = 0;   // ifのラベルにつけるユニークな値
    label_loop_count = 0; // loopのラベルにつけるユニークな値
    block_count = 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません。");
        return EXIT_FAILURE;
    }

    init();
    user_input = argv[1];

    token = tokenize(user_input);
    program();
    codegen();

    return EXIT_SUCCESS;
}