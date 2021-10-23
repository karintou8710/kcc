#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ベクターの定義 */
typedef struct Vector Vector;

struct Vector {
    void **body;
    int len;
    int capacity;
};

// 型の定義
typedef enum {
    TYPE_INT,
} TypeKind;

typedef struct Type Type;

struct Type {
    TypeKind kind;
};

/* トークンの定義 */
typedef struct Token Token;

typedef enum {
    TK_NUM = 256,  // number
    TK_IDENT,      // ident
    TK_EQ,         // ==
    TK_NE,         // !=
    TK_LE,         // <=
    TK_GE,         // >=
    TK_ADD_EQ,     // +=
    TK_SUB_EQ,     // -=
    TK_MUL_EQ,     // *=
    TK_DIV_EQ,     // /=
    TK_RETURN,     // return
    TK_IF,         // if
    TK_ELSE,       // else
    TK_FOR,        // for
    TK_WHILE,      // while
    TK_EOF,        // eof
    TK_TYPE,        // int
} TokenKind;

struct Token {
    TokenKind kind; //
    TypeKind type;  //
    Token *next;    //
    int val;        //
    char *str;      //
    int len;        //
};

Token *token;

/* ローカル変数の定義 */
typedef struct LVar LVar;

struct LVar {
    LVar *next; // 次の変数かNULL
    char *name; // 変数の名前
    int len;    // 名前の長さ
    int offset; // RBPからのオフセット
    Type *type;   // 型情報
};


/* ノードの定義 */ 

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
    ND_FOR,    // for
    ND_WHILE,  // while
    ND_BLOCK,  // block {}
    ND_CALL,   // call
    ND_ADDR,   // & アドレス
    ND_DEREF   // * ポインタ
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind; 
    Node *lhs;     // 左辺
    Node *rhs;     // 右辺
    int val;       // ND_NUMの時に使う
    LVar *lvar;    // kindがND_LVARの場合のみ使う
    char *fn_name; // 
    Vector *args;  // 
    Vector *stmts; // 

    // if (cond) then els 
    // while (cond) body 
    // for (init;cond;inc) body 
    Node *cond;    // 
    Node *then;    //
    Node *els;     //
    Node *body;    //
    Node *init;    //
    Node *inc;     //
};



/* 関数型の定義 */
typedef struct Function Function;

struct Function {
    char *name;
    Node *body;
    LVar *params;
    LVar *locals;
    int stack_size;
};

// parse.c
void program();

Node *new_node_num(int val);
Node *new_node(NodeKind kind);
Node *new_binop(NodeKind kind, Node *lhs, Node *rhs);
bool at_eof();
int expect_number();
void expect(int op);
bool consume(int op);
LVar *find_lvar(Token *tok);

// util.c
int is_alpha(char c);
int is_alnum(char c);
void str_advanve(char **p);
void next_token();
Token *get_next_token();
bool startsWith(char *p, char *q);
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
char *my_strndup(const char *s, size_t n);

Vector *new_vec();
void vec_push(Vector *v, void *elem);
void vec_pushi(Vector *v, int val);
void *vec_pop(Vector *v);
void *vec_last(Vector *v);
bool vec_contains(Vector *v, void *elem);
bool vec_union1(Vector *v, void *elem);

// codegen.c
void codegen();

// token.c
Token *tokenize(char *p);
Token *new_token(int kind, Token *cur, char *str, int len);

// 変数
char *user_input; // 入力プログラム
Function *funcs[100];
int label_if_count;
int label_loop_count;
int block_count;