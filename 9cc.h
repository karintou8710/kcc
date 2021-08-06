#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_RESERVED,
    TK_NUM,
    TK_IDENT,
    TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

Token *token;

typedef enum {
    ND_ADD,    // +
    ND_SUB,    // -
    ND_MUL,    // *
    ND_DIV,    // /
    ND_ASSIGN, // =
    ND_EQ,     // ==
    ND_NE,     // !=
    ND_LT,     // <
    ND_LE,     // <=
    ND_LVAR,   // local var
    ND_NUM,    // num
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
    int offset;    // kindがND_LVARの場合のみ使う
};

// 入力プログラム
char *user_input;

void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Node *new_node_num(int val);
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Token *tokenize(char *p);
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool at_eof();
int expect_number();
void expect(char *op);
bool consume(char *op);
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

void gen(Node *node);

Node *code[100];