#include "9cc.h"

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
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

bool startsWith(char *p, char *q) {
    return memcmp(p, q, strlen(q)) == 0;
}

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}

void str_advanve(char **p) {
    while ('a'<=**p && **p<='z') {
        *p += 1;
    }
}

void next_token() {
    token = token->next;
}

Vector *new_vec() {
    Vector *v = calloc(1, sizeof(Vector));
    v->body = calloc(16, sizeof(void *));
    v->capacity = 16;
    v->len = 0;
    return v;
}

void vec_push(Vector *v, void *elem) {
    if (v->len == v->capacity) {
        v->capacity *= 2;
        v->body = realloc(v->body, sizeof(void *) * v->capacity);
    }
    v->body[v->len++] = elem;
}

void vec_pushi(Vector *v, int val) {
    vec_push(v, &val);
}

void *vec_pop(Vector *v) {
  assert(v->len);
  return v->body[--v->len];
}

void *vec_last(Vector *v) {
  assert(v->len);
  return v->body[v->len - 1];
}

bool vec_contains(Vector *v, void *elem) {
  for (int i = 0; i < v->len; i++)
    if (v->body[i] == elem)
      return true;
  return false;
}

bool vec_union1(Vector *v, void *elem) {
  if (vec_contains(v, elem))
    return false;
  vec_push(v, elem);
  return true;
}