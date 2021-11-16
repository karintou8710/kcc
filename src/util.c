#include "9cc.h"

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

// エラー箇所を報告する
// TODO: 改行が必要なコード量になってきたので対応する
void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

bool startsWith(char *p, char *q)
{
    return memcmp(p, q, strlen(q)) == 0;
}

int is_alpha(char c)
{
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           (c == '_');
}

int is_alnum(char c)
{
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

// 変数の文字列分ポインタを進める
void str_advanve(char **p)
{
    while (is_alnum(**p))
    {
        *p += 1;
    }
}

void next_token()
{
    token = token->next;
}

Token *get_nafter_token(int n)
{
    Token *t = token;
    for (int i=0;i<n;i++) {
        t = t->next;
    }
    return t;
}

// n文字複製する
char *my_strndup(const char *s, size_t n)
{
    char *p;
    size_t n1;

    for (n1 = 0; n1 < n && s[n1] != '\0'; n1++)
        continue;
    p = malloc(n + 1);
    if (p != NULL)
    {
        memcpy(p, s, n1);
        p[n1] = '\0';
    }
    return p;
}

void swap(void **p, void **q)
{
    void *tmp = *p;
    *p = *q;
    *q = tmp;
}