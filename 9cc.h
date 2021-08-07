#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    TK_NUM = 256,  // number
    TK_IDENT,      // ident
    TK_EOF,        // eof
    TK_RETURN,     // return
    TK_IF,         // if
    TK_ELSE,       // else
    TK_EQ,        // ==
    TK_NE,        // !=
    TK_LE,        // <=
    TK_GE,        // >=
};

typedef struct Token Token;

struct Token {
    int kind;
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
    ND_RETURN, // return
    ND_IF,     // if
    ND_ELSE,   // else
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
    int offset;    // kindがND_LVARの場合のみ使う

    // if (cond) then els ...
    Node *cond; 
    Node *then;
    Node *els;
};

typedef struct LVar LVar;

// ローカル変数の型
struct LVar {
  LVar *next; // 次の変数かNULL
  char *name; // 変数の名前
  int len;    // 名前の長さ
  int offset; // RBPからのオフセット
};

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
Node *new_node(NodeKind kind);
Node *new_binop(NodeKind kind, Node *lhs, Node *rhs);
Token *tokenize(char *p);
Token *new_token(int kind, Token *cur, char *str, int len);
bool at_eof();
int expect_number();
void expect(int op);
bool consume(int op);
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
LVar *find_lvar(Token *tok);

// util.c
int is_alnum(char c);
void str_advanve(char **p);
void next_token();

// codegen.c
void gen(Node *node);

// 変数


LVar *locals; // ローカル変数
char *user_input; // 入力プログラム
Node *code[100];
int label_if_count;