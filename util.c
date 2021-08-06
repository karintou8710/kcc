#include "9cc.h"

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