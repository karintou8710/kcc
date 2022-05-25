#include "9cc.h"

Token *new_token(int kind, Token *cur, char *str, int len) {
    Token *tok = memory_alloc(sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;
    string_literal = new_vec();

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        // includeはスキップする
        if (strncmp(p, "#include", 8) == 0) {
            p += 8;
            while (*p != '\n') p++;
            continue;
        }

        if (strncmp(p, "static", 6) == 0) {
            p += 6;
            continue;
        }

        if (strncmp(p, "//", 2) == 0) {
            p += 2;
            while (*p != '\n') p++;
            continue;
        }

        if (strncmp(p, "/*", 2) == 0) {
            p += 2;
            char *q = strstr(p, "*/");
            if (!q) error_at(p - 2, "tokenize() failure: ブロックコメントが閉じられていません");
            p = q + 2;
            continue;
        }

        if (startsWith(p, "==")) {
            cur = new_token(TK_EQ, cur, p, 2);
            p += 2;
            continue;
        }

        if (startsWith(p, "!=")) {
            cur = new_token(TK_NE, cur, p, 2);
            p += 2;
            continue;
        }

        if (startsWith(p, "<=")) {
            cur = new_token(TK_LE, cur, p, 2);
            p += 2;
            continue;
        }

        if (startsWith(p, ">=")) {
            cur = new_token(TK_GE, cur, p, 2);
            p += 2;
            continue;
        }

        if (startsWith(p, "+=")) {
            cur = new_token(TK_ADD_EQ, cur, p, 2);
            p += 2;
            continue;
        }

        if (startsWith(p, "-=")) {
            cur = new_token(TK_SUB_EQ, cur, p, 2);
            p += 2;
            continue;
        }

        if (startsWith(p, "*=")) {
            cur = new_token(TK_MUL_EQ, cur, p, 2);
            p += 2;
            continue;
        }

        if (startsWith(p, "/=")) {
            cur = new_token(TK_DIV_EQ, cur, p, 2);
            p += 2;
            continue;
        }

        if (startsWith(p, "%=")) {
            cur = new_token(TK_MOD_EQ, cur, p, 2);
            p += 2;
            continue;
        }

        if (startsWith(p, "++")) {
            cur = new_token(TK_INC, cur, p, 2);
            p += 2;
            continue;
        }

        if (startsWith(p, "--")) {
            cur = new_token(TK_DEC, cur, p, 2);
            p += 2;
            continue;
        }

        if (startsWith(p, "&&")) {
            cur = new_token(TK_LOGICAL_AND, cur, p, 2);
            p += 2;
            continue;
        }

        if (startsWith(p, "||")) {
            cur = new_token(TK_LOGICAL_OR, cur, p, 2);
            p += 2;
            continue;
        }

        if (startsWith(p, "->")) {
            cur = new_token(TK_ARROW, cur, p, 2);
            p += 2;
            continue;
        }

        if (strchr("+-*/%=;()<>{},&[]!.?:|^", *p)) {
            cur = new_token(*p, cur, p, 1);
            p++;
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            cur->type = new_type(TYPE_INT);
            continue;
        }

        if (*p == '\"') {
            p++;
            cur = new_token(TK_STRING, cur, p, 0);
            char *q = p;
            int len = 0;
            while (*p && *p != '"') {
                len++;
                p++;
            }
            p++;
            cur->str = my_strndup(q, len);
            cur->len = p - q - 1;
            cur->str_literal_index = string_literal->len;
            vec_push(string_literal, cur);
            continue;
        }

        if (*p == '\'') {
            p++;
            cur = new_token(TK_NUM, cur, p, 1);
            cur->val = *p;
            p++;
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

        if (strncmp(p, "void", 4) == 0 && !is_alnum(p[4])) {
            cur = new_token(TK_TYPE, cur, p, 4);
            cur->type = new_type(TYPE_VOID);
            p += 4;
            continue;
        }

        if (strncmp(p, "struct", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_TYPE, cur, p, 6);
            cur->type = new_type(TYPE_STRUCT);
            p += 6;

            while (isspace(*p)) p++;

            if (is_alpha(*p)) {
                char *q = p;
                str_advanve(&p);
                cur->type->name = my_strndup(q, p - q);
                continue;
            } else {
                error_at(p, "tokenize() failure: structの型名が存在しません。");
            }
        }

        if (strncmp(p, "enum", 4) == 0 && !is_alnum(p[4])) {
            cur = new_token(TK_TYPE, cur, p, 4);
            cur->type = new_type(TYPE_ENUM);
            p += 4;

            while (isspace(*p)) p++;

            if (is_alpha(*p)) {
                char *q = p;
                str_advanve(&p);
                cur->type->name = my_strndup(q, p - q);
            }
            continue;
        }

        if (is_alpha(*p)) {
            cur = new_token(TK_IDENT, cur, p, 0);
            char *q = p;
            str_advanve(&p);
            cur->len = p - q;
            continue;
        }

        error_at(p, "tokenize() failure: トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}