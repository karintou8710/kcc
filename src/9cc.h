#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * ~ 構造体 ~
 * Vector, Type, Token, Var, Node, Function
 *
 */

typedef struct Var Var;
typedef struct Vector Vector;
typedef struct Type Type;
typedef struct Token Token;
typedef struct Node Node;
typedef struct Function Function;
typedef struct Initializer Initializer;
typedef struct GInit_el GInit_el;
typedef enum TypeKind TypeKind;
typedef enum NodeKind NodeKind;
typedef enum TokenKind TokenKind;

/* ベクターの定義 */

struct Vector {
    void **body;
    int len;
    int capacity;
};

/* 型の定義 */
enum TypeKind {
    TYPE_CHAR,
    TYPE_INT,
    TYPE_PTR,
    TYPE_ARRAY,
    TYPE_VOID,
    TYPE_STRUCT,
    TYPE_ENUM,
};

struct Type {
    TypeKind kind;
    Type *ptr_to;
    int size;
    int array_size;

    // struct, enum
    char *name;
    Var *member;
};

/* トークンの定義 */
enum TokenKind {
    TK_NUM = 256,    // number
    TK_IDENT,        // ident
    TK_EQ,           // ==
    TK_NE,           // !=
    TK_LE,           // <= 260
    TK_GE,           // >=
    TK_ADD_EQ,       // +=
    TK_SUB_EQ,       // -=
    TK_MUL_EQ,       // *=
    TK_INC,          // ++
    TK_DEC,          // --
    TK_DIV_EQ,       // /= 265
    TK_MOD_EQ,       // %=
    TK_RETURN,       // return
    TK_IF,           // if
    TK_ELSE,         // else
    TK_FOR,          // for
    TK_WHILE,        // while
    TK_EOF,          // eof
    TK_TYPE,         // int
    TK_SIZEOF,       // sizeof
    TK_STRING,       // string
    TK_CONTINUE,     // continue
    TK_BREAK,        // break
    TK_LOGICAL_AND,  // &&
    TK_LOGICAL_OR,   // ||
    TK_ARROW,        // ->
};

struct Token {
    TokenKind kind;         //
    Type *type;             //
    Token *next;            //
    int val;                //
    char *str;              //
    int str_literal_index;  //
    int len;                //
};

/* 変数・定数の定義 */
struct Var {
    Var *next;        // 次の変数かNULL
    char *name;       // 変数の名前
    int len;          // 名前の長さ
    int offset;       // RBPからのオフセット
    int next_offset;  // ローカルスコープでのオフセットを管理
    Type *type;       // 型情報
    int val;          // 定数の場合は値を持つ

    bool is_global;
    bool is_only_type;
    Vector *ginit;  // GInit_elのVector
};

/* ノードの定義 */
enum NodeKind {
    ND_ADD,            // +
    ND_SUB,            // -
    ND_MUL,            // *
    ND_DIV,            // /
    ND_MOD,            // %
    ND_ASSIGN,         // =
    ND_EQ,             // ==
    ND_NE,             // !=
    ND_LT,             // <
    ND_LE,             // <=
    ND_AND,            // &
    ND_OR,             // |
    ND_XOR,            // ^
    ND_VAR,            // local var
    ND_NUM,            // num
    ND_RETURN,         // return
    ND_IF,             // if
    ND_ELSE,           // else
    ND_FOR,            // for
    ND_WHILE,          // while
    ND_BLOCK,          // block {}
    ND_CALL,           // call
    ND_ADDR,           // & アドレス
    ND_DEREF,          // * ポインタ
    ND_STRING,         // string literal
    ND_CONTINUE,       // continue
    ND_BREAK,          // break
    ND_LOGICALNOT,     // !
    ND_LOGICAL_AND,    // &&
    ND_LOGICAL_OR,     // ||
    ND_SUGER,          // 糖衣構文
    ND_NULL,           // 何もしない
    ND_STRUCT_MEMBER,  // struct member
    ND_TERNARY,        // 3項演算子
};

struct Node {
    NodeKind kind;
    Node *lhs;          // 左辺
    Node *rhs;          // 右辺
    int val;            // ND_NUM ND_STRINGの時に使う
    Var *var;           // kindがND_VARの場合のみ使う
    char *fn_name;      //
    char *str_literal;  // ND_STRINGのときに使う
    Vector *args;       //
    Vector *stmts;      //
    Type *type;         // 型

    // if (cond) then els
    // while (cond) body
    // for (init;cond;inc) body
    Node *cond;
    Node *then;
    Node *els;
    Node *body;
    Node *init;
    Node *inc;
};

/* 関数型の定義 */
struct Function {
    char *name;
    Node *body;
    Var *params;
    Var *locals;
    int stack_size;

    Type *ret_type;  // return_type

    bool is_prototype;
};

struct Initializer {
    Type *type;
    Var *var;  // 代入先の変数を格納

    Node *expr;
    Initializer *children;
    int len;
};

// グローバル変数の初期化式
struct GInit_el {
    int val;
    char *str;
    int len;
};

// parse.c
void program();
Function *find_func(char *name);

// util.c
int is_alpha(char c);
int is_alnum(char c);
void str_advanve(char **p);
void next_token();
Token *get_nafter_token(int n);
bool startsWith(char *p, char *q);
void error_at(char *loc, char *msg);
void error(char *fmt, ...);
char *my_strndup(const char *s, size_t n);
void swap(void **p, void **q);
void *memory_alloc(size_t size);
void copy_func(Function *to, Function *from);

// debug.c
void print_node_kind(NodeKind kind);
void print_token_kind(TokenKind kind);
void print_type_kind(TypeKind kind);
void debug_var(Var *var);
void debug_type(Type *ty, int depth);
void debug_node(Node *node, char *pos, int depth);
void debug_token(Token *t);
void debug_initializer(Initializer *init, int depth);
void debug(char *fmt, ...);

// vector.c
Vector *new_vec();
void vec_push(Vector *v, void *elem);
void vec_pushi(Vector *v, int val);
void *vec_pop(Vector *v);
void *vec_last(Vector *v);
bool vec_contains(Vector *v, void *elem);
bool vec_union1(Vector *v, void *elem);
void vec_concat(Vector *to, Vector *from);
void *vec_delete(Vector *v, int index);

// codegen.c
void codegen();

// token.c
Token *tokenize(char *p);

// type.c
Type *new_type(TypeKind tykind);
Type *new_ptr_type(Type *ptr_to);
Type *new_array_type(Type *ptr_to, int size);
void add_type(Node *node);
int sizeOfType(Type *ty);
bool is_integertype(TypeKind kind);
TypeKind large_numtype(Type *t1, Type *t2);
bool can_type_cast(Type *ty, TypeKind to);
int array_base_type_size(Type *ty);
bool is_same_type(Type *ty1, Type *ty2);

// グローバル変数
Vector *string_literal;
Var *globals;
Token *token;      // tokenは単方向の連結リスト
char *user_input;  // 入力プログラム
char *file_name;
Vector *funcs;         // Function型のVector
int label_if_count;    // ifのラベル
int label_loop_count;  // forとwhileのラベル
Vector *struct_global_lists;
Vector *struct_local_lists;  // 既出の構造体
Vector *enum_global_lists;
Vector *enum_local_lists;  // 既出の列挙型