#include "9cc.h"

Token *new_token(int kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
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

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (startsWith(p, "==")) {
            cur = new_token(TK_EQ, cur, p, 2);
            p+=2;
            continue;
        }

        if (startsWith(p, "!=")) {
            cur = new_token(TK_NE, cur, p, 2);
            p+=2;
            continue;
        }

        if (startsWith(p, "<=")) {
            cur = new_token(TK_LE, cur, p, 2);
            p+=2;
            continue;
        }

        if (startsWith(p, ">=")) {
            cur = new_token(TK_GE, cur, p, 2);
            p+=2;
            continue;
        }

        if (strchr("+-*/=;()<>", *p)) {
            cur = new_token(*p, cur, p, 1);
            p++;
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
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

        // 小文字だけのローカル変数
        if ('a'<=*p && *p<='z') {
            cur = new_token(TK_IDENT, cur, p, 0);
            char *q = p;
            str_advanve(&p);
            cur->len = p - q;
            continue;
        }

        error("トークナイズできません");
    }

    
    new_token(TK_EOF, cur, p, 0);
    return head.next;
}