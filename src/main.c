#include "9cc.h"

void init() {
    label_if_count = 0;   // ifのラベルにつけるユニークな値
    label_loop_count = 0; // loopのラベルにつけるユニークな値
    globals = NULL; // グローバル変数の初期化
    struct_global_lists = new_vec();
    struct_local_lists = new_vec();
}

// 指定されたファイルの内容を返す
char *read_file(char *path) {
    // ファイルを開く
    FILE *fp = fopen(path, "r");
    if (!fp)
        error("cannot open %s: %s", path, strerror(errno));

    // ファイルの長さを調べる
    if (fseek(fp, 0, SEEK_END) == -1)
        error("%s: fseek: %s", path, strerror(errno));
    size_t size = ftell(fp);
    if (fseek(fp, 0, SEEK_SET) == -1)
        error("%s: fseek: %s", path, strerror(errno));

    // ファイル内容を読み込む
    char *buf = memory_alloc(size + 2);
    fread(buf, size, 1, fp);

    // ファイルが必ず"\n\0"で終わっているようにする
    if (size == 0 || buf[size - 1] != '\n')
        buf[size++] = '\n';
    buf[size] = '\0';
    fclose(fp);
    return buf;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません。\n");
        return EXIT_FAILURE;
    }

    init();
    file_name = argv[1];
    user_input = read_file(file_name);

    token = tokenize(user_input);
    program();
    codegen();

    return EXIT_SUCCESS;
}