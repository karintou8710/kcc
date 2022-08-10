#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Var Var;
typedef struct Vector Vector;
typedef struct Type Type;
typedef struct Tag Tag;
typedef struct Token Token;
typedef struct Node Node;
typedef struct Function Function;
typedef struct Initializer Initializer;
typedef struct GlobalInit GlobalInit;
typedef struct TypedefAlias TypedefAlias;
typedef enum TypeKind TypeKind;
typedef enum NodeKind NodeKind;
typedef enum TokenKind TokenKind;
typedef enum StorageClass StorageClass;

enum StorageClass {
    UNKNOWN,  // NULL。callcでzero初期化するために使う
    STORAGE_TYPEDEF,
    STORAGE_EXTERN,
};

/* 0 ~ 255は1文字のトークン */
enum TokenKind {
    TK_NUM = 256,    // number
    TK_IDENT,        // ident
    TK_EQ,           // ==
    TK_NE,           // !=
    TK_LE,           // <=
    TK_GE,           // >=
    TK_RSHIFT,       // >>
    TK_LSHIFT,       // <<
    TK_ADD_EQ,       // +=
    TK_SUB_EQ,       // -=
    TK_MUL_EQ,       // *=
    TK_DIV_EQ,       // /=
    TK_MOD_EQ,       // %=
    TK_AND_EQ,       // &=
    TK_OR_EQ,        // |=
    TK_XOR_EQ,       // ^=
    TK_LSHIFT_EQ,    // <<=
    TK_RSHIFT_EQ,    // >>=
    TK_INC,          // ++
    TK_DEC,          // --
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
    TK_TYPEDEF,      // typedef
    TK_VARIADIC,     // ...
    TK_INCLUDE,      // include
    TK_EXTERN,       // extern
    TK_DO,           // do
    TK_SWITCH,       // switch
    TK_CASE,         // case
    TK_DEFAULT,      // default
    TK_SIGNED,       // signed
    TK_UNSIGNED,     // unsigned (未実装)
    TK_CONST,        // const
};

enum TypeKind {
    TYPE_CHAR,
    TYPE_SHORT,
    TYPE_INT,
    TYPE_LONG,
    TYPE_PTR,
    TYPE_ARRAY,
    TYPE_VOID,
    TYPE_STRUCT,
    TYPE_UNION,
    TYPE_ENUM,
    TYPE_BOOL,
    TYPE_FUNC,
};

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
    ND_LSHIFT,         // <<
    ND_RSHIFT,         // >>
    ND_AND,            // &
    ND_OR,             // |
    ND_XOR,            // ^
    ND_NOT,            // ~
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
    ND_LOGICAL_NOT,    // !
    ND_LOGICAL_AND,    // &&
    ND_LOGICAL_OR,     // ||
    ND_SUGER,          // 糖衣構文
    ND_NULL,           // 何もしない
    ND_STRUCT_MEMBER,  // struct member
    ND_TERNARY,        // 3項演算子
    ND_CAST,           // キャスト
    ND_STMT_EXPR,      // stmt in expr
    ND_DO_WHILE,       // do ... while
    ND_SWITCH,         // switch
    ND_CASE,           // case
    ND_DEFAULT,        // default
};

struct Vector {
    void **body;
    int len;
    int capacity;
};

struct Type {
    TypeKind kind;
    Type *ptr_to;
    int size;        // 全体のサイズ
    int array_size;  // 配列の要素数
    size_t alignment;

    // struct, enum
    char *name;
    Var *member;
    bool is_forward;  // 前方宣言

    // unsigned const
    bool is_unsigned;
    bool is_constant;

    // nested type
    Token *token;

    // function
    Var *params;
    bool is_variadic;
};

/* TODO: 何故使うかコメント */
struct Tag {
    Type *base_type;  // 変数に使われる型構造体とは確保された領域を独立させる
    Vector *forward_type;
};

struct Token {
    TokenKind kind;
    Type *type;
    Token *next;
    long val;
    char *str;
    int len;                // 文字列の長さ
    int str_literal_index;  // 文字列リテラルの固有番号

    bool is_standard;  // 標準ヘッダーファイルのインクルード
};

/* TODO: next_offsetはoffsetの最大値さえあれば必要なさそう？ */
struct Var {
    Var *next;            // 次の変数かNULL
    char *name;           // 変数の名前
    int len;              // 変数名の長さ
    int offset;           // RBPからのオフセット
    int next_offset;      // ローカルスコープでのオフセットを管理
    Type *type;           // 型情報
    long val;             // 定数の場合は値を持つ
    Vector *global_init;  // GlobalInitのVector

    bool is_global;
    bool is_only_type;  // プロトタイプ宣言で使用
    bool is_extern;
};

struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    long val;
    Var *var;
    char *fn_name;
    char *str_literal;
    Type *type;
    Vector *args;  // 関数呼び出し時の引数
    Vector *stmts;

    // constで使用
    bool is_initialization;

    // case, labelで使用
    char *label_name;

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

struct Function {
    char *name;
    Node *body;
    Var *params;
    Var *locals;
    Var *va_area;  // 可変長引数のメモリ配置
    int stack_size;
    Type *ret_type;

    bool is_prototype;
    bool is_variadic;  // 可変長引数を引数に持つか
};

struct Initializer {
    Type *type;
    Var *var;  // 代入先の変数を格納

    Node *expr;
    Initializer *children;  // 配列として領域確保する
    int len;                // 配列長

    bool is_empty;  // 初期化子が足りていない場合
};

/* グローバル変数のコンパイル時計算用 */
struct GlobalInit {
    long val;
    char *str;  // ポインタの加減算
    int len;
};

/* Vectorでtypedefの対象を管理する */
struct TypedefAlias {
    char *name;
    Type *type;
};

// util.c
void assert(int n);
int is_alpha(char c);
int is_alnum(char c);
void str_advance(char **p);
void next_token();
Token *get_nth_token(int n);
bool starts_with(char *p, char *q);
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
char *my_strndup(char *s, size_t n);
void swap(void **p, void **q);
void *try_memory_allocation(size_t size);
void copy_func(Function *to, Function *from);

// debug.c
// TODO: print_~は文字列を返すようにする
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
void *vec_pop(Vector *v);
void *vec_last(Vector *v);
void vec_delete(Vector *v, int index);
bool vec_contains(Vector *v, void *elem);
bool vec_union(Vector *v, void *elem);
void vec_concat(Vector *to, Vector *from);

// type.c
Type *new_type(TypeKind tykind);
Type *new_ptr_type(Type *ptr_to);
Type *new_array_type(Type *ptr_to, int size);
void add_type(Node *node);
int sizeof_type(Type *ty);
size_t alignof_type(Type *ty);
int array_base_type_size(Type *ty);
void apply_align_to_struct(Type *ty);
bool is_integertype(TypeKind kind);
bool is_scalartype(TypeKind kind);
bool is_relationalnode(NodeKind kind);
TypeKind large_integer_type(Type *t1, Type *t2);
bool can_cast_type(Type *ty, TypeKind to);
bool is_same_type(Type *ty1, Type *ty2);
Tag *new_tag(Type *type);
void copy_type(Type *to, Type *from);
void shallowcopy_type(Type *to, Type *from);
void calc_type_size(Type *type);
Type *new_func_type(Type *ret_type, Var *params, bool *is_variadic, char *name);

// parse.c
void program();
Function *find_func(char *name);

// codegen.c
void codegen();

// token.c
Token *tokenize(char *p);

// preprocess.c
Token *preprocess(Token *tok);

// main.c
char *read_file(char *path);

// グローバル変数
char *user_input;         // 入力プログラム
char *file_name;          // 入力されたファイル名
int label_control_count;  // if, for, while, switch, 3項演算子のラベル番号
Var *globals;
Token *token;  // tokenは単方向の連結リスト
Vector *string_literal;
Vector *funcs;  // Function型のVector
Vector *struct_global_lists;
Vector *struct_local_lists;
Vector *union_global_lists;
Vector *union_local_lists;
Vector *enum_global_lists;
Vector *enum_local_lists;  // 既出の列挙型
Vector *typedef_alias;