#include "kcc.h"

static char escape_letters[][2] = {
    {'0', 0},
    {'a', 7},
    {'b', 8},
    {'e', 27},
    {'f', 12},
    {'n', 10},
    {'r', 13},
    {'t', 9},
    {'v', 12},
    {'\\', 92},
    {'\'', 39},
    {'"', 34},
    {'?', 63}};

static Token *new_token(int kind, Token *cur, char *str, int len) {
    Token *tok = try_memory_allocation(sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

static char escape_single_letter(char *p) {
    for (int i = 0; i < sizeof(escape_letters) / sizeof(char[2]); i++) {
        if (escape_letters[i][0] == *p) {
            return escape_letters[i][1];
        }
    }
    error("single_letter() failure: %dは対応していないエスケープシーケンスです。", *p);
}

static void skip_space(char **p) {
    char *q = *p;
    while (*q && isspace(*q)) q++;
    *p = q;
}

static void skip_to_target(char **p, char c) {
    char *q = *p;
    while (*q && *q != c) q++;
    *p = q;
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (strncmp(p, "#include", 8) == 0) {
            cur = new_token(TK_INCLUDE, cur, p, 8);
            p += 8;

            skip_space(&p);

            if (*p == '<') {
                // TODO: 標準ライブラリのインクルード
                cur->is_standard = true;
                while (*p != '\n') p++;
            } else if (*p == '"') {
                p++;

                char *q = p;
                skip_to_target(&p, '"');
                if (*p != '"') {
                    error("tokenize() failure: 「\"」で閉じていません。");
                }
                cur->str = my_strndup(q, p - q);
                cur->len = p - q + 1;

                p++;
            } else {
                error("tokenize() failure: #includeに失敗しました");
            }
            continue;
        }

        // プリプロセッサーが出力するメタ情報を削除
        if (starts_with(p, "#") && (user_input == p || *(p - 1) == '\n')) {
            while (*p != '\n') p++;
            continue;
        }

        // 未実装
        if (strncmp(p, "static", 6) == 0) {
            p += 6;
            continue;
        }

        if (strncmp(p, "//", 2) == 0) {
            p += 2;
            skip_to_target(&p, '\n');
            continue;
        }

        if (strncmp(p, "/*", 2) == 0) {
            p += 2;
            char *q = strstr(p, "*/");
            if (!q) error_at(p - 2, "tokenize() failure: ブロックコメントが閉じられていません");
            p = q + 2;
            continue;
        }

        if (starts_with(p, "...")) {
            cur = new_token(TK_VARIADIC, cur, p, 3);
            p += 3;
            continue;
        }

        if (starts_with(p, "<<=")) {
            cur = new_token(TK_LSHIFT_EQ, cur, p, 3);
            p += 3;
            continue;
        }

        if (starts_with(p, ">>=")) {
            cur = new_token(TK_RSHIFT_EQ, cur, p, 3);
            p += 3;
            continue;
        }

        if (starts_with(p, "==")) {
            cur = new_token(TK_EQ, cur, p, 2);
            p += 2;
            continue;
        }

        if (starts_with(p, "!=")) {
            cur = new_token(TK_NE, cur, p, 2);
            p += 2;
            continue;
        }

        if (starts_with(p, "<=")) {
            cur = new_token(TK_LE, cur, p, 2);
            p += 2;
            continue;
        }

        if (starts_with(p, ">=")) {
            cur = new_token(TK_GE, cur, p, 2);
            p += 2;
            continue;
        }

        if (starts_with(p, ">>")) {
            cur = new_token(TK_RSHIFT, cur, p, 2);
            p += 2;
            continue;
        }

        if (starts_with(p, "<<")) {
            cur = new_token(TK_LSHIFT, cur, p, 2);
            p += 2;
            continue;
        }

        if (starts_with(p, "+=")) {
            cur = new_token(TK_ADD_EQ, cur, p, 2);
            p += 2;
            continue;
        }

        if (starts_with(p, "-=")) {
            cur = new_token(TK_SUB_EQ, cur, p, 2);
            p += 2;
            continue;
        }

        if (starts_with(p, "*=")) {
            cur = new_token(TK_MUL_EQ, cur, p, 2);
            p += 2;
            continue;
        }

        if (starts_with(p, "/=")) {
            cur = new_token(TK_DIV_EQ, cur, p, 2);
            p += 2;
            continue;
        }

        if (starts_with(p, "%=")) {
            cur = new_token(TK_MOD_EQ, cur, p, 2);
            p += 2;
            continue;
        }

        if (starts_with(p, "&=")) {
            cur = new_token(TK_AND_EQ, cur, p, 2);
            p += 2;
            continue;
        }

        if (starts_with(p, "|=")) {
            cur = new_token(TK_OR_EQ, cur, p, 2);
            p += 2;
            continue;
        }

        if (starts_with(p, "^=")) {
            cur = new_token(TK_XOR_EQ, cur, p, 2);
            p += 2;
            continue;
        }

        if (starts_with(p, "++")) {
            cur = new_token(TK_INC, cur, p, 2);
            p += 2;
            continue;
        }

        if (starts_with(p, "--")) {
            cur = new_token(TK_DEC, cur, p, 2);
            p += 2;
            continue;
        }

        if (starts_with(p, "&&")) {
            cur = new_token(TK_LOGICAL_AND, cur, p, 2);
            p += 2;
            continue;
        }

        if (starts_with(p, "||")) {
            cur = new_token(TK_LOGICAL_OR, cur, p, 2);
            p += 2;
            continue;
        }

        if (starts_with(p, "->")) {
            cur = new_token(TK_ARROW, cur, p, 2);
            p += 2;
            continue;
        }

        if (strchr("+-*/%=;()<>{},&[]!.?:|^~", *p)) {
            cur = new_token(*p, cur, p, 1);
            p++;
            continue;
        }

        // 2進数
        if (starts_with(p, "0b")) {
            p += 2;
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 2);
            cur->len = p - q;
            if (is_alnum(*p)) {
                error_at(p, "tokenize() failure: 無効な2進数です");
            }

            cur->type = new_type(TYPE_INT);
            continue;
        }

        // 16進数
        if (starts_with(p, "0x")) {
            p += 2;
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 16);
            cur->len = p - q;
            if (is_alnum(*p)) {
                error_at(p, "tokenize() failure: 無効な16進数です");
            }
            cur->type = new_type(TYPE_INT);
            continue;
        }

        // 8進数
        if (starts_with(p, "0")) {
            p += 1;
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 8);
            cur->len = p - q;
            if (is_alnum(*p)) {
                error_at(p, "tokenize() failure: 無効な8進数です");
            }
            cur->type = new_type(TYPE_INT);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            if (is_alnum(*p)) {
                error_at(p, "tokenize() failure: 無効な10進数です");
            }
            cur->type = new_type(TYPE_INT);
            continue;
        }

        /*
         * 文字列リテラル
         * アセンブリにエスケープ文字を展開せずに出力する
         */
        if (*p == '\"') {
            p++;
            cur = new_token(TK_STRING, cur, p, 0);
            char *q = p;
            int len = 0;
            // 文字列の長さを取得する
            while (*p && *p != '"') {
                // \"があるのでエスケープ文字は飛ばす
                if (*p == '\\') {
                    p += 2;
                    len += 2;
                    continue;
                }
                len++;
                p++;
            }
            p++;
            cur->str = my_strndup(q, len);
            cur->len = strlen(cur->str);
            cur->str_literal_index = string_literal->len;
            vec_push(string_literal, cur);
            continue;
        }

        /* TODO: 複数文字のシングルクオーテーション */
        if (*p == '\'') {
            p++;
            if (*p == '\\') {
                cur = new_token(TK_NUM, cur, p, 2);
                p++;
                cur->val = escape_single_letter(p);
                p++;
            } else {
                cur = new_token(TK_NUM, cur, p, 1);
                cur->val = *p;
                p++;
            }
            if (*p != '\'') {
                error("tokenize() failure: 「'」で閉じていません。");
            }
            p++;
            continue;
        }

        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }

        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            cur = new_token(TK_IF, cur, p, 2);
            p += 2;
            continue;
        }

        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
            cur = new_token(TK_ELSE, cur, p, 4);
            p += 4;
            continue;
        }

        if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
            cur = new_token(TK_WHILE, cur, p, 5);
            p += 5;
            continue;
        }

        if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
            cur = new_token(TK_FOR, cur, p, 3);
            p += 3;
            continue;
        }

        if (strncmp(p, "switch", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_SWITCH, cur, p, 6);
            p += 6;
            continue;
        }

        if (strncmp(p, "break", 5) == 0 && !is_alnum(p[5])) {
            cur = new_token(TK_BREAK, cur, p, 5);
            p += 5;
            continue;
        }

        if (strncmp(p, "continue", 8) == 0 && !is_alnum(p[8])) {
            cur = new_token(TK_CONTINUE, cur, p, 8);
            p += 8;
            continue;
        }

        if (strncmp(p, "default", 7) == 0 && !is_alnum(p[7])) {
            cur = new_token(TK_DEFAULT, cur, p, 7);
            p += 7;
            continue;
        }

        if (strncmp(p, "case", 4) == 0 && !is_alnum(p[4])) {
            cur = new_token(TK_CASE, cur, p, 4);
            p += 4;
            continue;
        }

        if (strncmp(p, "sizeof", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_SIZEOF, cur, p, 6);
            p += 6;
            continue;
        }

        if (strncmp(p, "int", 3) == 0 && !is_alnum(p[3])) {
            cur = new_token(TK_TYPE, cur, p, 3);
            cur->type = new_type(TYPE_INT);
            p += 3;
            continue;
        }

        if (strncmp(p, "char", 4) == 0 && !is_alnum(p[4])) {
            cur = new_token(TK_TYPE, cur, p, 4);
            cur->type = new_type(TYPE_CHAR);
            p += 4;
            continue;
        }

        if (strncmp(p, "long", 4) == 0 && !is_alnum(p[4])) {
            cur = new_token(TK_TYPE, cur, p, 4);
            cur->type = new_type(TYPE_LONG);
            p += 4;
            continue;
        }

        if (strncmp(p, "short", 5) == 0 && !is_alnum(p[5])) {
            cur = new_token(TK_TYPE, cur, p, 5);
            cur->type = new_type(TYPE_SHORT);
            p += 5;
            continue;
        }

        if (strncmp(p, "void", 4) == 0 && !is_alnum(p[4])) {
            cur = new_token(TK_TYPE, cur, p, 4);
            cur->type = new_type(TYPE_VOID);
            p += 4;
            continue;
        }

        if (strncmp(p, "_Bool", 5) == 0 && !is_alnum(p[5])) {
            cur = new_token(TK_TYPE, cur, p, 5);
            cur->type = new_type(TYPE_BOOL);
            p += 5;
            continue;
        }

        if (strncmp(p, "signed", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_SIGNED, cur, p, 6);
            p += 6;
            continue;
        }

        if (strncmp(p, "unsigned", 8) == 0 && !is_alnum(p[8])) {
            cur = new_token(TK_UNSIGNED, cur, p, 8);
            p += 8;
            continue;
        }

        if (strncmp(p, "const", 5) == 0 && !is_alnum(p[5])) {
            cur = new_token(TK_CONST, cur, p, 5);
            p += 5;
            continue;
        }

        if (strncmp(p, "struct", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_TYPE, cur, p, 6);
            cur->type = new_type(TYPE_STRUCT);
            p += 6;

            skip_space(&p);

            if (is_alpha(*p)) {
                char *q = p;
                str_advance(&p);
                cur->type->name = my_strndup(q, p - q);
                continue;
            } else {
                /* 無名構造体 */
                continue;
            }
        }

        if (strncmp(p, "union", 5) == 0 && !is_alnum(p[5])) {
            cur = new_token(TK_TYPE, cur, p, 5);
            cur->type = new_type(TYPE_UNION);
            p += 5;

            skip_space(&p);

            if (is_alpha(*p)) {
                char *q = p;
                str_advance(&p);
                cur->type->name = my_strndup(q, p - q);
                continue;
            } else {
                /* 匿名共用体 */
                continue;
            }
        }

        if (strncmp(p, "enum", 4) == 0 && !is_alnum(p[4])) {
            cur = new_token(TK_TYPE, cur, p, 4);
            cur->type = new_type(TYPE_ENUM);
            p += 4;

            skip_space(&p);

            if (is_alpha(*p)) {
                char *q = p;
                str_advance(&p);
                cur->type->name = my_strndup(q, p - q);
            }
            continue;
        }

        if (strncmp(p, "typedef", 7) == 0 && !is_alnum(p[7])) {
            cur = new_token(TK_TYPEDEF, cur, p, 7);
            p += 7;
            continue;
        }

        if (strncmp(p, "extern", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_EXTERN, cur, p, 6);
            p += 6;
            continue;
        }

        if (strncmp(p, "do", 2) == 0 && !is_alnum(p[2])) {
            cur = new_token(TK_DO, cur, p, 2);
            p += 2;
            continue;
        }

        if (is_alpha(*p)) {
            cur = new_token(TK_IDENT, cur, p, 0);
            char *q = p;
            str_advance(&p);
            cur->len = p - q;
            continue;
        }

        error_at(p, "tokenize() failure: トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}