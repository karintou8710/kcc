#include "kcc.h"

/*
 * ローカル変数を単方向の連結リストで持つ
 * localsは後に出たローカル変数のポインタを持つ
 */
static Var *locals;
static Function *cur_parse_func;
static Vector *local_scope;
static bool is_global = true;
/* cast, defaultの条件をswitchで始めに生成するのでswitchノードに持つ */
static Node *node_in_switch;
/* case,defaultのラベル番号 */
static int switch_label_cnt;
static StorageClass current_storage = UNKNOWN;

static Type *find_typedef_alias(char *name);

/* nodeの生成 */
static Node *new_add(Node *lhs, Node *rhs);
static Node *new_sub(Node *lhs, Node *rhs);
static Node *new_mul(Node *lhs, Node *rhs);
static Node *new_div(Node *lhs, Node *rhs);
static Node *new_mod(Node *lhs, Node *rhs);
static Node *new_cast(Node *lhs, Type *to_type);
static Node *new_node_num(long val);
static Var *new_lvar(Token *tok, Type *type);
static Var *new_gvar(Token *tok, Type *type);
static void create_lvar_from_params(Var *params);
static Var *find_lvar(Token *tok);
static Var *find_gvar(Token *tok);
static Vector *new_node_init2(Initializer *init, Node *node);

/* AST */
static Type *declaration_specifier();
static Type *type_specifier(int *flag);
static Var *enumerator(Type *type, int *enum_const_num);
static void enumerator_list(Type *type);
static Node *initialize(Initializer *init, Node *node);
static void initialize2(Initializer *init);
static Node *declaration_global(Type *type);
static Node *declaration_var(Type *type);
static Node *declaration(Type *type);
static Type *declarator(Type *type);
static Type *declarator2(Type *type);
static Node *declarator_var(Type *type);
static void declarator_struct(Type *member_type, Type *struct_type);
static Type *abstruct_declarator(Type *type);
static Var *declaration_param(Var *cur);
static Var *declaration_params(bool *is_variadic);
static Type *pointer(Type *type);
static void func_define(Type *type);
Type *struct_declaration(Type *type);
static Node *compound_stmt();
static Node *stmt();
static Node *labeld();
static Node *expr();
static Node *assign();
static Node *conditional();
static Node *logical_or();
static Node *logical_and();
static Node *inclusive_or();
static Node *exclusive_or();
static Node *bin_and();
static Node *equality();
static Node *relational();
static Node *shift();
static Node *add();
static Node *mul();
static Node *cast();
static Node *unary();
static Node *postfix();
static Type *type_suffix(Type *type, bool is_first);
static Node *primary();
static long const_expr();
static GInitEl *eval(Node *node);

enum {
    VOID = 1 << 0,
    CHAR = 1 << 1,
    SHORT = 1 << 2,
    INT = 1 << 3,
    LONG = 1 << 4,
    LONGLONG = 1 << 5,
    STRUCT = 1 << 6,
    UNION = 1 << 7,
    ENUM = 1 << 8,
    TYPEDEF = 1 << 9,
    BOOL = 1 << 10,
    SIGNED = 1 << 11,
    UNSIGNED = 1 << 12,  // 未実装
    CONST = 1 << 13,
};

/*** parser utils ***/

/* 指定された演算子が来る可能性がある */
static bool consume(int op) {
    if (token->kind != op) {
        return false;
    }
    next_token();
    return true;
}

static bool consume_nostep(int op) {
    if (token->kind != op) {
        return false;
    }
    return true;
}

// typedefに対応
static bool consume_is_type_nostep(Token *tok) {
    if (tok->kind == TK_TYPE) return true;

    if (tok->kind == TK_IDENT) {
        char *name = my_strndup(tok->str, tok->len);
        if (find_typedef_alias(name) != NULL) return true;
    }

    // 型の修飾子や指定子
    TokenKind type_tokens[] = {
        TK_EXTERN, TK_TYPEDEF, TK_SIGNED, TK_UNSIGNED, TK_CONST};

    for (int i = 0; i < sizeof(type_tokens) / sizeof(TokenKind); i++) {
        if (tok->kind == type_tokens[i]) return true;
    }

    return false;
}

/* 指定された演算子が必ず来る */
static void expect(int op) {
    if (token->kind != op) {
        if (op == TK_TYPE) {
            error_at(token->str, "expect() failure: 適当な位置に型がありません");
        }
        char msg[100];
        snprintf(msg, 100, "expect() failure: 「%c」ではありません。", op);
        error_at(token->str, msg);
    }
    next_token();
}

static void expect_nostep(int op) {
    if (token->kind != op) {
        if (op == TK_TYPE) {
            error_at(token->str, "expect_nostep() failure: 適当な位置に型がありません");
        }

        char msg[100];
        snprintf(msg, 100, "expect_nostep() failure: 「%c」ではありません。", op);
        error_at(token->str, msg);
    }
}

static long expect_number() {
    if (token->kind != TK_NUM) {
        error_at(token->str, "expect_number() failure: 数値ではありません");
    }
    long val = token->val;
    next_token();
    return val;
}

static void expect_no_storage() {
    if (current_storage != UNKNOWN) {
        error("expect_no_storage() failure: 記憶子が複数定義されています");
    }
}

static bool at_eof() {
    return token->kind == TK_EOF;
}

// ポインターを読み進めて関数かグローバル変数か判断
static bool is_func(Token *tok) {
    while (tok->kind == '*') {
        tok = tok->next;
    }

    if (tok->kind != TK_IDENT)
        return false;
    tok = tok->next;
    return tok->kind == '(';
}

/*** new objects ***/
static Var *new_lvar(Token *tok, Type *type) {
    Var *lvar = memory_alloc(sizeof(Var));
    lvar->name = my_strndup(tok->str, tok->len);
    lvar->len = tok->len;
    lvar->type = type;
    if (locals->next_offset > 0) {
        lvar->offset = locals->next_offset + sizeOfType(type);
    } else {
        lvar->offset = locals->offset + sizeOfType(type);
    }

    /*
     * typedefはローカル変数を生成しない
     * コードの都合上変数自体は生成する
     */
    if (current_storage != STORAGE_TYPEDEF) {
        lvar->next = locals;
        locals = lvar;  // localsを新しいローカル変数に更新
    }

    return lvar;
}

static Var *new_gvar(Token *tok, Type *type) {
    Var *gvar = memory_alloc(sizeof(Var));
    gvar->name = my_strndup(tok->str, tok->len);
    gvar->len = tok->len;
    gvar->type = type;
    gvar->is_global = true;
    gvar->ginit = new_vec();

    /*
     * typedefはグローバル変数を生成しない
     * コードの都合上変数自体は生成する
     */
    if (current_storage != STORAGE_TYPEDEF) {
        gvar->next = globals;
        globals = gvar;  // globalsを新しいグローバル変数に更新
    }

    return gvar;
}

static void new_struct_member(Token *tok, Type *member_type, Type *struct_type) {
    Var *member = memory_alloc(sizeof(Var));
    member->next = struct_type->member;
    member->name = my_strndup(tok->str, tok->len);
    member->len = tok->len;
    member->type = member_type;
    // offsetはalignmentを考慮するので後で決める。unionは0。
    member->offset = 0;
    struct_type->member = member;
    struct_type->size += member_type->size;
}

static Var *new_enum_member(Token *tok, Type *type, int enum_const_num) {
    Var *var = memory_alloc(sizeof(Var));
    var->name = my_strndup(tok->str, tok->len);
    var->len = tok->len;
    var->val = enum_const_num;
    var->type = type;
    return var;
}

static TypedefAlias *new_typedef_alias(char *name, Type *type) {
    TypedefAlias *ta = memory_alloc(sizeof(TypedefAlias));
    ta->name = name;
    ta->type = type;
    return ta;
}

static Initializer *new_initializer(Var *var) {
    Initializer *init = memory_alloc(sizeof(Initializer));
    init->type = var->type;
    init->var = var;
    return init;
}

/*** find ***/

static Var *find_var(char *name, Var *vars, Var *end) {
    if (name == NULL) return NULL;
    if (vars == NULL) return NULL;

    for (Var *var = vars; var != end; var = var->next) {
        if (!var->name) continue;
        if (strcmp(name, var->name) == 0) {
            return var;
        }
    }
    return NULL;
}

/* ローカル変数取得時に使用 */
static Var *find_lvar(Token *tok) {
    char *name = my_strndup(tok->str, tok->len);
    return find_var(name, locals, NULL);
}

/* ローカル変数定義時に使用 */
static Var *find_scope_lvar(Token *tok) {
    char *name = my_strndup(tok->str, tok->len);
    return find_var(name, locals, vec_last(local_scope));
}

/* 既に定義されたグローバル変数を検索 */
static Var *find_gvar(Token *tok) {
    char *name = my_strndup(tok->str, tok->len);
    return find_var(name, globals, NULL);
}

static Var *find_allscope_var(Token *tok) {
    Var *var = find_lvar(tok);  // ローカル変数を取得
    if (!var) {
        var = find_gvar(tok);  // グローバル変数から取得
        if (!var) {
            return NULL;
        }
    }
    return var;
}

static Var *find_params(char *name, Var *params) {
    return find_var(name, params, NULL);
}

static Tag *find_tag(char *name, Vector *list) {
    if (name == NULL) return NULL;
    if (list == NULL) return NULL;

    for (int i = 0; i < list->len; i++) {
        Tag *tag = list->body[i];
        Type *t = tag->base_type;
        if (t->name == NULL) continue;
        if (strcmp(t->name, name) == 0) {
            return tag;
        }
    }

    return NULL;
}

static Tag *find_lstruct_type(char *name) {
    return find_tag(name, struct_local_lists);
}

static Tag *find_gstruct_type(char *name) {
    return find_tag(name, struct_global_lists);
}

static Tag *find_lunion_type(char *name) {
    return find_tag(name, union_local_lists);
}

static Tag *find_gunion_type(char *name) {
    return find_tag(name, union_global_lists);
}

static Tag *find_lenum_type(char *name) {
    return find_tag(name, enum_local_lists);
}

static Tag *find_genum_type(char *name) {
    return find_tag(name, enum_global_lists);
}

static Tag *find_enum_type(char *name) {
    // ローカルで探索
    Tag *t = find_lenum_type(name);
    if (t) return t;

    // グローバルで探索
    t = find_genum_type(name);
    return t;
}

// enum型のメンバーを探索
static Var *find_lenum_member(char *name) {
    if (name == NULL) return NULL;
    for (int i = 0; i < enum_local_lists->len; i++) {
        Tag *tag = enum_local_lists->body[i];
        Type *t = tag->base_type;
        Var *v = find_var(name, t->member, NULL);
        if (v) return v;
    }

    return NULL;
}

static Var *find_genum_member(char *name) {
    if (name == NULL) return NULL;
    for (int i = 0; i < enum_global_lists->len; i++) {
        Tag *tag = enum_global_lists->body[i];
        Type *t = tag->base_type;
        Var *v = find_var(name, t->member, NULL);
        if (v) return v;
    }

    return NULL;
}

static Var *find_enum_member(Token *tok) {
    char *name = my_strndup(tok->str, tok->len);
    // ローカルで探索
    Var *v = find_lenum_member(name);
    if (v) return v;

    // グローバルで探索
    v = find_genum_member(name);
    return v;
}

static Type *find_typedef_alias(char *name) {
    if (name == NULL) return NULL;
    for (int i = 0; i < typedef_alias->len; i++) {
        TypedefAlias *ta = typedef_alias->body[i];
        if (!ta->name) continue;
        if (strcmp(ta->name, name) == 0) {
            return ta->type;
        }
    }

    return NULL;
}

Function *find_func(char *name) {
    if (name == NULL) return NULL;
    for (int i = 0; i < funcs->len; i++) {
        Function *fn = funcs->body[i];
        if (!fn->name) continue;
        if (strcmp(fn->name, name) == 0) {
            return fn;
        }
    }

    return NULL;
}

/*** eval ***/

static void eval_concat(GInitEl *g, GInitEl *gl, GInitEl *gr, char *op, int len) {
    int max_digit = 50;
    if (gl->str && gr->str) {
        error("eval_concat() failure: オペランドが不適切です [%s]", op);
    } else if (gl->str && !gr->str) {
        if (strcmp(op, "+") != 0 && strcmp(op, "-") != 0) {
            error("eval_concat() failure: オペランドが不適切です [%s]", op);
        }
        int buf_size = gl->len + max_digit + len + 1;
        char *buf = memory_alloc(sizeof(char) * buf_size);
        buf_size = snprintf(buf, buf_size, "%s %s %ld", gl->str, op, gr->val);
        g->str = buf;
        g->len = buf_size;
    } else if (!gl->str && gr->str) {
        if (strcmp(op, "+") != 0 && strcmp(op, "-") != 0) {
            error("eval_concat() failure: オペランドが不適切です [%s]", op);
        }
        int buf_size = max_digit + gr->len + len + 1;
        char *buf = memory_alloc(sizeof(char) * buf_size);
        buf_size = snprintf(buf, buf_size, "%ld %s %s", gl->val, op, gr->str);
        g->str = buf;
        g->len = buf_size;
    } else {
        if (strcmp(op, "+") == 0) {
            g->val = gl->val + gr->val;
        } else if (strcmp(op, "-") == 0) {
            g->val = gl->val - gr->val;
        } else if (strcmp(op, "*") == 0) {
            g->val = gl->val * gr->val;
        } else if (strcmp(op, "/") == 0) {
            g->val = gl->val / gr->val;
        } else if (strcmp(op, "%") == 0) {
            g->val = gl->val % gr->val;
        } else if (strcmp(op, "==") == 0) {
            g->val = gl->val == gr->val;
        } else if (strcmp(op, "!=") == 0) {
            g->val = gl->val != gr->val;
        } else if (strcmp(op, "<") == 0) {
            g->val = gl->val < gr->val;
        } else if (strcmp(op, "<=") == 0) {
            g->val = gl->val <= gr->val;
        } else if (strcmp(op, "!") == 0) {
            g->val = !gl->val;
        } else if (strcmp(op, "&&") == 0) {
            g->val = gl->val && gr->val;
        } else if (strcmp(op, "||") == 0) {
            g->val = gl->val || gr->val;
        } else if (strcmp(op, "<<") == 0) {
            g->val = gl->val << gr->val;
        } else if (strcmp(op, ">>") == 0) {
            g->val = gl->val >> gr->val;
        }
    }
}

/* TODO: 四則演算以外にも対応 */
static GInitEl *eval(Node *node) {
    GInitEl *g = memory_alloc(sizeof(GInitEl));
    add_type(node);

    if (node->kind == ND_NUM) {
        g->val = node->val;
        return g;
    } else if (node->kind == ND_ADD) {
        GInitEl *gl = eval(node->lhs);
        GInitEl *gr = eval(node->rhs);
        eval_concat(g, gl, gr, "+", 2);
        return g;
    } else if (node->kind == ND_SUB) {
        GInitEl *gl = eval(node->lhs);
        GInitEl *gr = eval(node->rhs);
        g->val = gl->val - gr->val;
        eval_concat(g, gl, gr, "-", 2);
        return g;
    } else if (node->kind == ND_MUL) {
        GInitEl *gl = eval(node->lhs);
        GInitEl *gr = eval(node->rhs);
        eval_concat(g, gl, gr, "*", 2);
        return g;
    } else if (node->kind == ND_DIV) {
        GInitEl *gl = eval(node->lhs);
        GInitEl *gr = eval(node->rhs);
        eval_concat(g, gl, gr, "/", 2);
        return g;
    } else if (node->kind == ND_MOD) {
        GInitEl *gl = eval(node->lhs);
        GInitEl *gr = eval(node->rhs);
        eval_concat(g, gl, gr, "%", 2);
        return g;
    } else if (node->kind == ND_EQ) {
        GInitEl *gl = eval(node->lhs);
        GInitEl *gr = eval(node->rhs);
        eval_concat(g, gl, gr, "==", 3);
        return g;
    } else if (node->kind == ND_NE) {
        GInitEl *gl = eval(node->lhs);
        GInitEl *gr = eval(node->rhs);
        eval_concat(g, gl, gr, "!=", 3);
        return g;
    } else if (node->kind == ND_LT) {
        GInitEl *gl = eval(node->lhs);
        GInitEl *gr = eval(node->rhs);
        eval_concat(g, gl, gr, "<", 2);
        return g;
    } else if (node->kind == ND_LE) {
        GInitEl *gl = eval(node->lhs);
        GInitEl *gr = eval(node->rhs);
        eval_concat(g, gl, gr, "<=", 3);
        return g;
    } else if (node->kind == ND_LOGICAL_NOT) {
        GInitEl *gl = eval(node->lhs);
        GInitEl *gr = memory_alloc(sizeof(GInitEl));
        eval_concat(g, gl, gr, "!", 2);
        return g;
    } else if (node->kind == ND_LOGICAL_AND) {
        GInitEl *gl = eval(node->lhs);
        GInitEl *gr = eval(node->rhs);
        eval_concat(g, gl, gr, "&&", 3);
        return g;
    } else if (node->kind == ND_LOGICAL_OR) {
        GInitEl *gl = eval(node->lhs);
        GInitEl *gr = eval(node->rhs);
        eval_concat(g, gl, gr, "||", 3);
        return g;
    } else if (node->kind == ND_STRING) {
        int buf_size = 50;  // 仮の値を決める
        char *buf = memory_alloc(sizeof(char) * buf_size);
        buf_size = snprintf(buf, buf_size, ".LC%ld", node->val);
        g->str = buf;
        g->len = buf_size;
        return g;
    } else if (node->kind == ND_ADDR) {
        g->str = my_strndup(node->lhs->var->name, node->lhs->var->len);
        g->len = node->lhs->var->len;
        return g;
    } else if (node->kind == ND_VAR && node->type->kind == TYPE_ARRAY) {
        // 配列はポインター型として扱う
        g->str = my_strndup(node->var->name, node->var->len);
        g->len = node->var->len;
        return g;
    } else if (node->kind == ND_SUGER || node->kind == ND_STMT_EXPR) {
        for (int i = 0; i < node->stmts->len; i++) {
            Node *n = node->stmts->body[i];
            g = eval(n);
        }
        return g;
    } else if (node->kind == ND_LSHIFT) {
        GInitEl *gl = eval(node->lhs);
        GInitEl *gr = eval(node->rhs);
        eval_concat(g, gl, gr, "<<", 2);
        return g;
    } else if (node->kind == ND_RSHIFT) {
        GInitEl *gl = eval(node->lhs);
        GInitEl *gr = eval(node->rhs);
        eval_concat(g, gl, gr, ">>", 2);
        return g;
    }

    error("eval() failure: %d:未対応のNodeタイプです", node->kind);
}

/*** other func ***/

static Tag *is_defined_enum_type(char *name) {
    if (is_global) {
        Tag *tag = find_genum_type(name);
        if (tag && tag->forward_type->len == 0) {
            error("find_genum_type() failure: %s列挙型は既に宣言済みです。", name);
        }
        // 前方宣言
        return tag;
    } else {
        Tag *tag = find_lenum_type(name);
        if (tag && tag->forward_type->len == 0) {
            error("find_lenum_type() failure: %s列挙型は既に宣言済みです。", name);
        }
        // 前方宣言
        return tag;
    }
}

/* ノードを引数にもつsizeofの実装 */
static int sizeOfNode(Node *node) {
    add_type(node);
    return sizeOfType(node->type);
}

static void start_local_scope() {
    vec_push(local_scope, locals);
}

static void end_local_scope() {
    Var *var = vec_pop(local_scope);
    var->next_offset = locals->next_offset > 0 ? locals->next_offset : locals->offset;
    locals = var;
}

// TODO: 引数に適切な型をつけるようにする
/* 引数からローカル変数を作成する(前から見ていく) */
static void create_lvar_from_params(Var *params) {
    if (!params)
        return;

    Var *lvar = memory_alloc(sizeof(Var));
    lvar->name = params->name;
    lvar->len = params->len;
    lvar->type = params->type;
    // 引数はトップのローカルスコープになるのでnext_offsetで条件分岐が必要ない
    lvar->offset = locals->offset + sizeOfType(lvar->type);
    lvar->next = locals;

    locals = lvar;
    create_lvar_from_params(params->next);
}

static bool is_already_defined_global_obj(Token *tok) {
    char *name = my_strndup(tok->str, tok->len);
    return find_gvar(tok) != NULL;
}

static bool is_same_params(Var *params1, Var *params2) {
    for (Var *v1 = params1, *v2 = params2;; v1 = v1->next, v2 = v2->next) {
        if (v1 == NULL || v2 == NULL) {
            // NULL == NULL -> params1とparams2は等しい
            return v1 == v2;
        }

        if (!is_same_type(v1->type, v2->type)) {
            return false;
        }
    }
}

static void should_cast_args(Vector *args, Var *params, bool is_variadic) {
    if (args == NULL) error("should_cast_params() failure: args == NULL");

    int args_len = args->len;
    int params_len = 0;
    Var *tmp_p = params;
    while (tmp_p) {
        params_len++;
        tmp_p = tmp_p->next;
    }

    if ((is_variadic && args_len < params_len) ||
        (!is_variadic && args_len != params_len)) {
        error("should_cast_params() failure: length error args=%d params=%d is_variadic=%d", args_len, params_len, is_variadic);
    }

    int i = 0;
    /* args_len >= params_lenとなる */
    while (i < params_len) {
        Node *n = args->body[i];
        Var *v = params;

        if (!can_type_cast(n->type, v->type->kind)) {
            error("should_cast_params() failure: cast error %d %d", n->type->kind, v->type->kind);
        }

        if (!is_same_type(n->type, v->type))
            args->body[i] = new_cast(n, v->type);

        params = params->next;
        i++;
    }
}

static bool has_lvar_in_all_params(Var *params) {
    for (Var *v = params; v; v = v->next) {
        if (v->is_only_type) return false;
    }
    return true;
}

static Var *reverse_linked_list_var(Var *cur) {
    Var *reversed = NULL;
    while (cur) {
        Var *tmp = cur->next;
        cur->next = reversed;
        reversed = cur;
        cur = tmp;
    }
    return reversed->next;
}

static Tag *struct_defined_or_forward(Type *type) {
    if (is_global) {
        Tag *tag = find_gstruct_type(type->name);
        if (tag && tag->forward_type->len == 0) {
            error("find_gstruct_type() failure: %s構造体は既に宣言済みです。", type->name);
        }
        return tag;
    } else {
        Tag *tag = find_lstruct_type(type->name);
        if (tag && tag->forward_type->len == 0) {
            error("find_lstruct_type() failure: %s構造体は既に宣言済みです。", type->name);
        }
        return tag;
    }
}

static Tag *union_defined_or_forward(Type *type) {
    if (is_global) {
        Tag *tag = find_gunion_type(type->name);
        if (tag && tag->forward_type->len == 0) {
            error("find_gunion_type() failure: %s構造体は既に宣言済みです。", type->name);
        }
        return tag;
    } else {
        Tag *tag = find_lunion_type(type->name);
        if (tag && tag->forward_type->len == 0) {
            error("find_lunion_type() failure: %s構造体は既に宣言済みです。", type->name);
        }
        return tag;
    }
}

/*************************************/
/******                         ******/
/******        NEW_NODE         ******/
/******                         ******/
/*************************************/

// ノード作成
static Node *new_node(NodeKind kind) {
    Node *node = memory_alloc(sizeof(Node));
    node->kind = kind;
    return node;
}

// 演算子ノード作成
static Node *new_binop(NodeKind kind, Node *lhs, Node *rhs) {
    add_type(lhs);
    add_type(rhs);
    Node *node = memory_alloc(sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

/*
 * 演算子には型のキャストがある
 */
static Node *new_add(Node *lhs, Node *rhs) {
    // 型を伝搬する
    add_type(lhs);
    add_type(rhs);

    // enum TypeKind の順番にする (lhs <= rhs)
    if (lhs->type->kind > rhs->type->kind) {
        swap((void **)&lhs, (void **)&rhs);
    }

    Node *node = new_binop(ND_ADD, lhs, rhs);

    if (is_integertype(lhs->type->kind) && is_integertype(rhs->type->kind)) {
        return node;
    }

    if (is_integertype(lhs->type->kind) && rhs->type->kind == TYPE_PTR) {
        node->lhs = new_mul(lhs, new_node_num(rhs->type->ptr_to->size));
        add_type(node->lhs);
        return node;
    }

    if (is_integertype(lhs->type->kind) && rhs->type->kind == TYPE_ARRAY) {
        // ポインター型として演算
        node->lhs = new_mul(lhs, new_node_num(rhs->type->ptr_to->size));
        add_type(node->lhs);
        return node;
    }

    error("new_add() failure: (lhs=%d rhs=%d) 実行できない型による演算です", lhs->type->kind, rhs->type->kind);
}

static Node *new_sub(Node *lhs, Node *rhs) {
    add_type(lhs);
    add_type(rhs);

    // lhsとrhsの順番に関係あり
    Node *node = new_binop(ND_SUB, lhs, rhs);

    if (is_integertype(lhs->type->kind) && is_integertype(rhs->type->kind)) {
        return node;
    }

    if (lhs->type->kind == TYPE_PTR && is_integertype(rhs->type->kind)) {
        node->rhs = new_mul(rhs, new_node_num(lhs->type->ptr_to->size));
        add_type(node->rhs);
        return node;
    }

    if (lhs->type->kind == TYPE_ARRAY && is_integertype(rhs->type->kind)) {
        // ポインター型として演算
        node->rhs = new_mul(rhs, new_node_num(lhs->type->ptr_to->size));
        add_type(node->rhs);
        return node;
    }

    if (lhs->type->kind == TYPE_PTR && rhs->type->kind == TYPE_PTR ||
        lhs->type->kind == TYPE_ARRAY && rhs->type->kind == TYPE_ARRAY ||
        lhs->type->kind == TYPE_ARRAY && rhs->type->kind == TYPE_PTR ||
        lhs->type->kind == TYPE_PTR && rhs->type->kind == TYPE_ARRAY) {
        if (lhs->type->ptr_to->size != rhs->type->ptr_to->size) {
            error("new_sub() failure: 異なるサイズの型を指すポインター同士の引き算はできません。");
        }
        add_type(node);
        node = new_div(node, new_node_num(lhs->type->ptr_to->size));
        return node;
    }

    error("new_sub() failure: 実行できない型による演算です");
}

static Node *new_mul(Node *lhs, Node *rhs) {
    add_type(lhs);
    add_type(rhs);

    // enum TypeKind の順番にする (lhs <= rhs)
    if (lhs->type->kind > rhs->type->kind) {
        swap((void **)&lhs, (void **)&rhs);
    }

    Node *node = new_binop(ND_MUL, lhs, rhs);

    if (is_integertype(lhs->type->kind) && is_integertype(rhs->type->kind)) {
        return node;
    }

    error("new_mul() failure: 実行できない型による演算です");
}

static Node *new_div(Node *lhs, Node *rhs) {
    add_type(lhs);
    add_type(rhs);

    // lhsとrhsの順番に関係あり (lhs <= rhs)
    Node *node = new_binop(ND_DIV, lhs, rhs);

    if (is_integertype(lhs->type->kind) && is_integertype(rhs->type->kind)) {
        return node;
    }

    error("new_div() failure: 実行できない型による演算です");
}

static Node *new_mod(Node *lhs, Node *rhs) {
    add_type(lhs);
    add_type(rhs);

    // lhsとrhsの順番に関係あり (lhs <= rhs)
    Node *node = new_binop(ND_MOD, lhs, rhs);

    if (is_integertype(lhs->type->kind) && is_integertype(rhs->type->kind)) {
        return node;
    }

    error("new_mod() failure: 実行できない型による演算です");
}

static Node *new_assign(Node *lhs, Node *rhs) {
    add_type(lhs);
    add_type(rhs);

    Node *node = new_cast(rhs, lhs->type);
    node = new_binop(ND_ASSIGN, lhs, node);
    // 代入できるかチェック
    add_type(node);

    return node;
}

static Node *new_cast(Node *lhs, Type *to_type) {
    Node *node = new_node(ND_CAST);
    node->lhs = lhs;
    node->type = to_type;
    return node;
}

/* 数値ノードを作成 */
static Node *new_node_num(long val) {
    Node *node = new_node(ND_NUM);
    node->val = val;
    return node;
}

/* 初期化式からノードを作成
 * int a[2][2] = {{1,2},{3,4}};
 *        ↓↓↓
 * a[0][0] = 1;
 * a[0][1] = 2;
 * a[1][0] = 3;
 * a[1][1] = 4;
 */
static Node *new_node_init(Initializer *init, Node *node) {
    Node *n = new_node(ND_SUGER);
    n->stmts = new_node_init2(init, node);
    return n;
}

static Vector *new_node_init2(Initializer *init, Node *node) {
    Vector *suger = new_vec();

    if (init->children) {
        if (init->type->kind == TYPE_STRUCT) {
            int i = 0;
            for (Var *member = init->type->member; member; member = member->next) {
                Node *n = new_node(ND_STRUCT_MEMBER);
                n->lhs = node;
                n->val = member->offset;
                n->type = member->type;
                Vector *v = new_node_init2(init->children + i, n);
                vec_concat(suger, v);
                i++;
            }
        } else {
            // array, string
            for (int i = 0; i < init->len; i++) {
                Node *deref = new_node(ND_DEREF);
                deref->lhs = new_add(node, new_node_num(i));
                add_type(deref->lhs);
                add_type(deref);
                Vector *v = new_node_init2(init->children + i, deref);
                vec_concat(suger, v);
            }
        }

        return suger;
    }

    if (is_global) {
        vec_push(init->var->ginit, eval(init->expr));
    } else {
        Node *n = new_assign(node, init->expr);
        n->is_initialize = true;
        vec_push(suger, n);
    }

    return suger;
}

/* 変数を宣言 */
static Node *declear_node_ident(Token *tok, Type *type) {
    Node *node = new_node(ND_VAR);
    bool f = is_global ? is_already_defined_global_obj(tok) : find_scope_lvar(tok) != NULL;
    if (f) {
        error_at(tok->str, "declear_node_ident() failure: 既に宣言済みです");
    }

    Var *var = is_global ? new_gvar(tok, type) : new_lvar(tok, type);
    node->var = var;
    return node;
}

// 変数のノードを取得
static Node *should_new_node_var(Token *tok) {
    Node *node = new_node(ND_VAR);
    node->var = find_allscope_var(tok);
    if (node->var == NULL) {
        error("");
    }
    return node;
}

static int count_var(Var *v) {
    int cnt = 0;
    while (v) {
        cnt++;
        v = v->next;
    }
    return cnt;
}

/*************************************/
/******                         ******/
/******           AST           ******/
/******                         ******/
/*************************************/

/*
 *  <program> = ( <declaration> | <func_define> )*
 */
void program() {
    local_scope = new_vec();
    while (!at_eof()) {
        Type *type = declaration_specifier();
        if (is_func(token)) {
            is_global = false;
            func_define(type);
            is_global = true;
        } else {
            if ((type->kind == TYPE_STRUCT || type->kind == TYPE_UNION || type->kind == TYPE_ENUM) &&
                consume(';')) {
                // 構造体・列強型の作成 or 宣言
                continue;
            }
            declaration(type);
        }
        current_storage = UNKNOWN;
    }
}

/*
 *  <declaration> = <declaration_specifier> <declaration_var> ("," <declaration_var>)* ";"
 */
static Node *declaration(Type *type) {
    Node *node = declaration_var(type);
    if (consume(';')) {
        if (node->kind == ND_VAR && current_storage == STORAGE_TYPEDEF) {
            /* typedefの型を使う側でコピーする */
            TypedefAlias *ta = new_typedef_alias(node->var->name, node->var->type);
            vec_push(typedef_alias, ta);
        } else if (node->kind == ND_VAR && current_storage == STORAGE_EXTERN) {
            node->var->is_extern = true;
        }
        return node;
    }

    Node *n = new_node(ND_SUGER);
    n->stmts = new_vec();
    vec_push(n->stmts, node);
    while (consume(',')) {
        vec_push(n->stmts, declaration_var(type));
    }

    for (int i = 0; i < n->stmts->len; i++) {
        Node *tmp_node = n->stmts->body[i];
        // typedefなら型名を記録する
        if (tmp_node->kind == ND_VAR && current_storage == STORAGE_TYPEDEF) {
            /* typedefの型を使う側でコピーする */
            TypedefAlias *ta = new_typedef_alias(tmp_node->var->name, tmp_node->var->type);
            vec_push(typedef_alias, ta);
        } else if (tmp_node->kind == ND_VAR && current_storage == STORAGE_EXTERN) {
            tmp_node->var->is_extern = true;
        }
    }
    current_storage = UNKNOWN;

    expect(';');
    return n;
}

/*
 *  <declaration_var> = <declarator> ("=" <initialize>)?
 */
static Node *declaration_var(Type *type) {
    Node *node = declarator_var(type);

    // 変数
    if (consume('=')) {
        if (current_storage == STORAGE_TYPEDEF) {
            error("declaration_var() failure: typedef中で初期化はできません");
        } else if (current_storage == STORAGE_EXTERN) {
            error("declaration_var() failure: extern中で初期化はできません");
        }

        Initializer *init = new_initializer(node->var);
        return initialize(init, node);
    }

    if (node->var->type->kind == TYPE_ARRAY && sizeOfType(node->var->type) == 0) {
        error("declaration_var() failure: 宣言で配列の添え字は省略できません");
    }

    return node;
}

/*
 *  <initialize> = <assign>
 *               | "{" <initialize> ("," <initialize>)* "}"
 */
static Node *initialize(Initializer *init, Node *node) {
    bool is_index_omitted = node->var->type->kind == TYPE_ARRAY && node->var->type->array_size == 0;
    initialize2(init);
    if (is_index_omitted) node->var->offset += sizeOfType(node->var->type);
    return new_node_init(init, node);
}

static void initialize_array(Initializer *init) {
    Type *ty = init->type;

    if (ty->array_size == 0) {
        // 最初の添え字が省略されている
        // TODO: Vectorで実装していない理由の調査
        int children_cap = 2;
        init->children = memory_alloc(sizeof(Initializer) * children_cap);
        expect('{');
        int i = 0;
        while (!consume('}')) {
            if (i == children_cap) {
                // cannot use realloc() here, since the memory has to be zero-cleared in order for this compiler to work
                Initializer *new_p = memory_alloc(sizeof(Initializer) * children_cap * 2);
                memcpy(new_p, init->children, sizeof(Initializer) * children_cap);
                children_cap *= 2;
                init->children = new_p;
            }

            (init->children + i)->var = init->var;
            if (i > 0) expect(',');
            (init->children + i)->type = ty->ptr_to;
            initialize2(init->children + i);
            i++;
        }
        // 省略された配列の長さとサイズを決定
        init->len = ty->array_size = i;
        ty->size = ty->array_size * ty->ptr_to->size;
    } else {
        init->children = memory_alloc(sizeof(Initializer) * ty->array_size);
        init->len = ty->array_size;

        // 初期化子がない場合は無視する
        if (!init->is_empty) expect('{');

        for (int i = 0; i < init->len; i++) {
            (init->children + i)->var = init->var;
            if (init->is_empty || consume_nostep('}')) {
                // 初期化子が省略
                (init->children + i)->is_empty = true;
                (init->children + i)->type = ty->ptr_to;
                initialize2(init->children + i);
                continue;
            }

            if (i > 0) expect(',');
            (init->children + i)->type = ty->ptr_to;
            initialize2(init->children + i);
        }
        // 初期化子がない場合は無視する
        if (!init->is_empty) expect('}');
    }
}

static void initialize_string(Initializer *init) {
    Type *ty = init->type;

    if (ty->array_size == 0) {
        // 省略された配列の長さとサイズを決定
        ty->array_size = token->len + 1;
        ty->size = ty->array_size * ty->ptr_to->size;
    }
    init->children = memory_alloc(sizeof(Initializer) * ty->array_size);
    init->len = ty->array_size;
    if (init->len < token->len) {
        error("initialize_string() failure: 文字列が長すぎます");
    }
    for (int i = 0; i < init->len; i++) {
        (init->children + i)->var = init->var;
        if (init->is_empty || token->len <= i) {
            // 初期化子が省略
            (init->children + i)->is_empty = true;
            (init->children + i)->type = ty->ptr_to;
            initialize2(init->children + i);
            continue;
        }
        (init->children + i)->type = ty->ptr_to;
        (init->children + i)->expr = new_node_num(token->str[i]);
    }
    next_token();
}

static void initialize_struct(Initializer *init) {
    Type *ty = init->type;

    if (ty->member == NULL) {
        error("initialize_struct() failure: 未定義の構造体です");
    }

    int member_num = count_var(ty->member);
    init->children = memory_alloc(sizeof(Initializer) * member_num);
    init->len = member_num;

    // 初期化子がない場合は無視する
    if (!init->is_empty) expect('{');

    int i = 0;
    Var *v = ty->member;
    while (i < init->len) {
        (init->children + i)->var = init->var;
        if (init->is_empty || consume_nostep('}')) {
            // 初期化子が省略
            (init->children + i)->is_empty = true;
            (init->children + i)->type = v->type;
            initialize2(init->children + i);
            i++;
            v = v->next;
            continue;
        }

        if (i > 0) expect(',');
        (init->children + i)->type = v->type;
        initialize2(init->children + i);

        i++;
        v = v->next;
    }
    // 初期化子がない場合は無視する
    if (!init->is_empty) expect('}');
}

static void initialize2(Initializer *init) {
    if (init->type->kind == TYPE_ARRAY) {
        // string
        if (init->type->ptr_to->kind == TYPE_CHAR && consume_nostep(TK_STRING)) {
            initialize_string(init);
            return;
        }

        // array
        initialize_array(init);
        return;
    }

    if (init->type->kind == TYPE_STRUCT && consume_nostep('{')) {
        initialize_struct(init);
        return;
    }

    if (init->is_empty) {
        init->expr = new_node_num(0);
    } else {
        init->expr = assign();
    }
}

/*
 *  <pointer> = ("*" <type_qualifier>?) *
 */
static Type *pointer(Type *type) {
    while (consume('*')) {
        Type *t = new_ptr_type(type);
        if (consume(TK_CONST)) {
            t->is_constant = true;
        }
        type = t;
    }
    return type;
}

/*
 * <declarator> = <pointer> <ident> <type_suffix>
 *              | <pointer> "(" <declarator> ")" <type_suffix>
 * <abstruct_declarator> = <pointer> <type_suffix>
 *                       | <pointer> "(" <declarator> ")" <type_suffix>
 * この２つの切り替えはdeclarator関数の呼び出し側でtype->tokenの有無で判定する
 */
static Type *declarator(Type *type) {
    type = declarator2(type);
    calc_type_size(type);

    return type;
}

static Type *declarator2(Type *type) {
    type = pointer(type);
    if (consume('(')) {
        // ex) int (*a)[10]; | * -> [10] -> int
        Type *dummy = memory_alloc(sizeof(Type));
        // ex) * -> dummy
        Type *head = declarator(dummy);
        expect(')');
        // ex) [10] -> int
        type = type_suffix(type, true);
        // ex) dummy = [10]
        copy_type_shallow(dummy, type);
        return head;
    } else {
        if (consume_nostep(TK_IDENT)) {
            Token *tok = token;
            next_token();
            type = type_suffix(type, true);
            type->token = tok;
        } else {
            type = type_suffix(type, true);
            type->token = NULL;
        }

        return type;
    }
}

static Node *declarator_var(Type *type) {
    type = declarator(type);
    if (type->token == NULL) {
        error("declarator() failure: type->token == NULL");
    }
    Node *node = declear_node_ident(type->token, type);
    // 新しい型のオフセットにする

    node->var->offset += sizeOfType(node->var->type);
    return node;
}

static void declarator_struct(Type *member_type, Type *struct_type) {
    member_type = declarator(member_type);
    if (member_type->token == NULL) {
        error("declarator() failure: member_type->token == NULL");
    }
    new_struct_member(member_type->token, member_type, struct_type);
}

/*
 * <declaration_param> = <declaration_specifier> (<abstruct_declarator> | <declarator>)
 */
static Var *declarator_param(Type *type, Var *cur) {
    type = declarator(type);

    Var *lvar = memory_alloc(sizeof(Var));
    lvar->type = type;
    if (type->token == NULL) {
        lvar->is_only_type = true;
    } else {
        lvar->name = my_strndup(type->token->str, type->token->len);
        lvar->len = type->token->len;
    }

    // 新しい型のオフセットにする
    lvar->offset = cur->offset + sizeOfType(lvar->type);
    return lvar;
}

/*
 * <abstruct_declarator> = <pointer> <type_suffix>
 */
static Type *abstruct_declarator(Type *type) {
    type = declarator(type);
    if (type->token) {
        error("declarator() failure: type->token has value");
    }
    return type;
}

/*
 * <declaration_specifier> = (<storage_class> | <type_specifier> | <type_qualifier>)+
 * <storage_class>  = "typedef" | "entern"
 * <type_qualifier> = "const"
 */
static Type *declaration_specifier() {
    if (!consume_is_type_nostep(token)) {
        error("declaration_specifier() failure: expect type");
    }
    Type *type = NULL;
    int flag = 0;

    while (consume_is_type_nostep(token)) {
        // storage class
        if (consume(TK_TYPEDEF)) {
            expect_no_storage();
            current_storage = STORAGE_TYPEDEF;
            continue;
        } else if (consume(TK_EXTERN)) {
            expect_no_storage();
            current_storage = STORAGE_EXTERN;
            continue;
        }

        // type_qualifier
        if (consume(TK_CONST)) {
            flag |= CONST;
            continue;
        }

        // TODO: 不正な型の例外処理
        Type *t = type_specifier(&flag);
        if (t) type = t;
    }

    if (flag & LONG || flag & LONGLONG) {
        type = new_type(TYPE_LONG);
    }

    if (flag & UNSIGNED) {
        // 未実装
        type->is_unsigned = true;
    }

    if (flag & CONST) {
        Type *t = type;
        while (t->kind == TYPE_ARRAY) t = t->ptr_to;
        t->is_constant = true;
    }

    return type;
}

/*
 * <type_specifier> = "char"
 *                  | "short"
 *                  | "int"
 *                  | "long"
 *                  | "void"
 *                  | "_Bool"
 *                  | ("struct" | "union") <ident>
 *                  | ("struct" | "union") <ident> "{" <struct_declaration>* "}"
 *                  | "enum" <ident>
 *                  | "enum" <ident>? "{" <enumerator_list> "}"
 *                  | "signed"
 *                  | "unsigned"
 */
static Type *type_specifier(int *flag) {
    Token *tok = token;
    Type *type;

    if (consume(TK_IDENT)) {
        // typedef
        *flag |= TYPEDEF;
        char *name = my_strndup(tok->str, tok->len);
        Type *t = find_typedef_alias(name);
        if (t == NULL) {
            error("find_typedef_alias() failure: %sは定義されていません", name);
        }
        type = memory_alloc(sizeof(Type));
        copy_type(type, t);
        if (type->is_forward) {
            Tag *stag_local, *stag_global;
            if (type->kind == TYPE_STRUCT) {
                stag_local = find_lstruct_type(type->name), stag_global = find_gstruct_type(type->name);
            } else if (type->kind == TYPE_UNION) {
                stag_local = find_lunion_type(type->name), stag_global = find_gunion_type(type->name);
            } else if (type->kind == TYPE_ENUM) {
                stag_local = find_lenum_type(type->name), stag_global = find_genum_type(type->name);
            } else {
                error("type_specifier() failure: 不正な前方宣言の型です");
            }
            Tag *stag = stag_local ? stag_local : stag_global;
            if (stag == NULL) {
                error("find_~_type() failure: 前方宣言の構造体がありません");
            }
            vec_push(stag->forward_type, type);
        }
    } else if (consume(TK_TYPE)) {
        // 基礎型
        type = tok->type;
        switch (tok->type->kind) {
            case TYPE_VOID:
                *flag |= VOID;
                break;
            case TYPE_CHAR:
                *flag |= CHAR;
                break;
            case TYPE_SHORT:
                *flag |= SHORT;
                break;
            case TYPE_INT:
                *flag |= INT;
                break;
            case TYPE_LONG:
                if (*flag & LONG) {
                    *flag |= LONGLONG;
                } else {
                    *flag |= LONG;
                }
                break;
            case TYPE_BOOL:
                *flag |= BOOL;
                break;
            case TYPE_STRUCT:
                *flag |= STRUCT;
                break;
            case TYPE_UNION:
                *flag |= UNION;
                break;
            case TYPE_ENUM:
                *flag |= ENUM;
                break;
        }
    } else if (consume(TK_SIGNED)) {
        *flag |= SIGNED;
        return NULL;
    } else if (consume(TK_UNSIGNED)) {
        *flag |= UNSIGNED;
        return NULL;
    } else {
        error("type_specifier() failure: 適切な型ではありません");
    }

    if (type->kind == TYPE_STRUCT && consume('{')) {
        // 構造体のメンバーの宣言
        type->member = memory_alloc(sizeof(Var));
        while (!consume('}')) {
            type = struct_declaration(type);
        }
        // 定義した順に並べ直す
        type->member = reverse_linked_list_var(type->member);
        // memberのアライメントを考慮したoffsetを決定する
        apply_align_struct(type);

        Tag *tag = struct_defined_or_forward(type);

        if (tag) {
            /* 前方宣言とlistに保存した型に反映する */
            for (int i = 0; i < tag->forward_type->len; i++) {
                Type *forward_type = tag->forward_type->body[i];
                forward_type->is_forward = false;
                copy_type(forward_type, type);
            }
            copy_type(tag->base_type, type);
            tag->forward_type = new_vec();  // 全て削除
        } else {
            tag = new_tag(type);
            vec_push(is_global ? struct_global_lists : struct_local_lists, tag);
        }

        return type;
    }

    if (type->kind == TYPE_UNION && consume('{')) {
        // 構造体のメンバーの宣言
        type->member = memory_alloc(sizeof(Var));
        while (!consume('}')) {
            type = struct_declaration(type);
        }
        // 定義した順に並べ直す
        type->member = reverse_linked_list_var(type->member);

        apply_align_struct(type);

        Tag *tag = union_defined_or_forward(type);

        if (tag) {
            /* 前方宣言とlistに保存した型に反映する */
            for (int i = 0; i < tag->forward_type->len; i++) {
                Type *forward_type = tag->forward_type->body[i];
                forward_type->is_forward = false;
                copy_type(forward_type, type);
            }
            copy_type(tag->base_type, type);
            tag->forward_type = new_vec();  // 全て削除
        } else {
            tag = new_tag(type);
            vec_push(is_global ? union_global_lists : union_local_lists, tag);
        }

        return type;
    }

    if (type->kind == TYPE_ENUM && consume('{')) {
        enumerator_list(type);

        // is_defined_enum_typeは列挙型が既に定義されていたら、強制終了する
        Tag *tag = is_defined_enum_type(type->name);

        if (tag) {
            /* 前方宣言とlistに保存した型に反映する */
            for (int i = 0; i < tag->forward_type->len; i++) {
                Type *forward_type = tag->forward_type->body[i];
                forward_type->is_forward = false;
                copy_type(forward_type, type);
            }
            copy_type(tag->base_type, type);
            tag->forward_type = new_vec();  // 全て削除
        } else {
            tag = new_tag(type);
            vec_push(is_global ? enum_global_lists : enum_local_lists, tag);
        }

        return type;
    }

    if (type->kind == TYPE_STRUCT) {
        Tag *stag_local = find_lstruct_type(type->name), *stag_global = find_gstruct_type(type->name);
        Tag *stag = stag_local ? stag_local : stag_global;
        if (stag == NULL) {
            // 初の前方宣言
            type->is_forward = true;
            stag = new_tag(type);
            vec_push(stag->forward_type, type);
            vec_push(is_global ? struct_global_lists : struct_local_lists, stag);
        } else {
            if (stag->forward_type->len > 0) {
                // 前方宣言・宣言済み
                vec_push(stag->forward_type, type);
            } else {
                copy_type(type, stag->base_type);  // base_typeに変更が反映されないようコピーする
            }
        }
    }

    if (type->kind == TYPE_UNION) {
        Tag *stag_local = find_lunion_type(type->name), *stag_global = find_gunion_type(type->name);
        Tag *stag = stag_local ? stag_local : stag_global;
        if (stag == NULL) {
            // 初の前方宣言
            type->is_forward = true;
            stag = new_tag(type);
            vec_push(stag->forward_type, type);
            vec_push(is_global ? union_global_lists : union_local_lists, stag);
        } else {
            if (stag->forward_type->len > 0) {
                // 前方宣言・宣言済み
                vec_push(stag->forward_type, type);
            } else {
                copy_type(type, stag->base_type);  // base_typeに変更が反映されないようコピーする
            }
        }
    }

    if (type->kind == TYPE_ENUM) {
        Tag *stag = find_enum_type(type->name);
        if (stag == NULL) {
            // 初の前方宣言
            type->is_forward = true;
            stag = new_tag(type);
            vec_push(stag->forward_type, type);
            vec_push(is_global ? enum_global_lists : enum_local_lists, stag);
        } else {
            if (stag->forward_type->len > 0) {
                // 前方宣言・宣言済み
                vec_push(stag->forward_type, type);
            } else {
                copy_type(type, stag->base_type);  // base_typeに変更が反映されないようコピーする
            }
        }
    }

    return type;
}

/*
 * <type_name> = <declaration_specifier> <abstruct_declarator>
 */
Type *type_name() {
    Type *type = declaration_specifier();
    type = abstruct_declarator(type);
    return type;
}

// is_firstは配列の初期化時のみ使用
/*
 *  <type_suffix> = "[" <const_expr>? "]" <type_suffix> | ε
 */
static Type *type_suffix(Type *type, bool is_first) {
    if (consume('[')) {
        int array_size;
        if (is_first && consume(']')) {
            array_size = 0;
        } else {
            array_size = const_expr();
            expect(']');
        }
        type = new_array_type(type_suffix(type, false), array_size);
    }

    if (consume('(')) {
        Type *type_func = new_type(TYPE_FUNC);
        type_func->ptr_to = type;
        type = type_func;
        expect(')');
    }

    return type;
}

/*
 *  <struct_declaration> = <declaration_specifier> <declarator> ";"
 */
Type *struct_declaration(Type *type) {
    Type *t = declaration_specifier();
    declarator_struct(t, type);
    expect(';');
    return type;
}

/*
 * <enumerator_list> = <enumerator> (",", <enumerator>)* ","?
 */
static void enumerator_list(Type *type) {
    int enum_const_num = 0;
    Var *var = enumerator(type, &enum_const_num);
    consume(',');

    while (!consume('}')) {
        Var *tmp_var = enumerator(type, &enum_const_num);
        tmp_var->next = var;
        var = tmp_var;
        consume(',');
    }
    type->member = var;
}

/*
 * <enumerator> = <ident>
 *              | <ident> "=" <conditional>
 */
static Var *enumerator(Type *type, int *enum_const_num) {
    Token *t = token;
    expect(TK_IDENT);
    Var *var = new_enum_member(t, type, *enum_const_num);
    *enum_const_num += 1;

    if (consume('=')) {
        GInitEl *el = eval(conditional());
        if (el->str) {
            error("enumerator() failure: 数値型の定数ではありません");
        }
        var->val = el->val;
        *enum_const_num = el->val + 1;
    }
    return var;
}

/*
 *  <declaration_param> = <declaration_specifier> (<abstruct_declarator> | <declarator>)
 */
static Var *declaration_param(Var *cur) {
    Type *type = declaration_specifier();
    return declarator_param(type, cur);
}

/*
 * <declaration_params> = "(" (<declaration_param> ("," <declaration_param>)* ("," "...")? | "void" | ε)  ")"
 */
static Var *declaration_params(bool *is_variadic) {
    expect('(');

    // (void)の場合
    if (token->kind == TK_TYPE &&
        token->type->kind == TYPE_VOID &&
        get_nafter_token(1)->kind == ')') {
        expect(TK_TYPE);
    }

    Var head = {};
    Var *cur = &head;  // 引数の単方向連結リスト

    while (!consume(')')) {
        if (cur != &head) {
            expect(',');
        }

        if (consume(TK_VARIADIC)) {
            if (cur == &head) {
                error("declaration_params() failure: ...は第一引数に設定できません");
            }

            *is_variadic = true;
            expect(')');
            break;
        }

        Var *p = declaration_param(cur);
        // 配列型は暗黙にポインターとして扱う
        if (p->type->kind == TYPE_ARRAY) {
            Type *t = p->type;
            p->type = new_ptr_type(t->ptr_to);
            p->offset += sizeOfType(p->type) - sizeOfType(t);
        }

        // 変数名の重複チェック
        if (p->name) {
            char *name = my_strndup(p->name, p->len);
            if (find_params(name, head.next)) {
                error("find_params() error: 既に%sは定義されています", name);
            }
        }

        cur = cur->next = p;
    }

    return head.next;
}

/*
 *  <func_define> = <declaration_specifier> <pointer> <ident> <declaration_params> <compound_stmt>
 */
static void func_define(Type *type) {
    type = pointer(type);
    Function *fn = memory_alloc(sizeof(Function));
    cur_parse_func = fn;

    Token *tok = token;
    bool is_variadic = false;

    expect(TK_IDENT);
    fn->name = my_strndup(tok->str, tok->len);
    if (find_gvar(tok)) {
        error("func_define() failure: 既に%sは定義されています", fn->name);
    }
    fn->ret_type = type;

    fn->params = declaration_params(&is_variadic);  // 前から見ていく
    fn->is_variadic = is_variadic;

    Function *entry = find_func(fn->name);
    if (consume(';')) {
        // プロトタイプ宣言
        fn->is_prototype = true;
        if (entry == NULL) {
            vec_push(funcs, fn);
            return;
        }

        if (!is_same_params(fn->params, entry->params) ||
            !is_same_type(fn->ret_type, entry->ret_type)) {
            error("func_define() failure: 異なる型でのプロトタイプ宣言です");
        }

        return;
    }

    // 定義
    if (entry) {
        if (!entry->is_prototype) {
            error("func_define() failure: 既に%sは定義されています", fn->name);
        }

        if (!is_same_params(fn->params, entry->params) ||
            !is_same_type(fn->ret_type, entry->ret_type)) {
            error("func_define() failure: %sは異なる型での宣言です", fn->name);
        }
    }

    // 定義では型だけの引数を許容しない
    if (!has_lvar_in_all_params(fn->params)) {
        error("has_lvar_in_all_params() failure: 引数の定義には変数名が必要です");
    }

    /* 再帰関数用に先に登録する */
    vec_push(funcs, fn);

    /* 関数をグローバル変数として登録する */
    Type *fn_type = new_func_type(fn);
    new_gvar(tok, fn_type);

    locals = memory_alloc(sizeof(Var));
    struct_local_lists = new_vec();  // 関数毎に構造体を初期化
    union_local_lists = new_vec();
    enum_local_lists = new_vec();
    locals->offset = 0;
    start_local_scope();
    create_lvar_from_params(fn->params);

    // 可変長引数
    if (fn->is_variadic) {
        Token *t = memory_alloc(sizeof(Token));
        t->str = "__va_area__";
        t->len = strlen(t->str);
        fn->va_area = new_lvar(t, new_array_type(new_type(TYPE_CHAR), 136));
    }

    fn->body = compound_stmt();
    end_local_scope();
    fn->locals = locals;
    return;
}

/*
 *  <compound_stmt> = "{" (<declaration> | <stmt>)* "}"
 */
static Node *compound_stmt() {
    expect('{');
    // ローカルのスコープを取る為に、現在のローカル変数を保持
    start_local_scope();
    Node *node = new_node(ND_BLOCK);
    node->stmts = new_vec();
    while (!consume('}')) {
        Node *n;
        if (consume_is_type_nostep(token)) {
            Type *type = declaration_specifier();
            if ((type->kind == TYPE_STRUCT || type->kind == TYPE_UNION || type->kind == TYPE_ENUM) &&
                consume_nostep(';')) {
                // 構造体か列挙型の作成 or 宣言
                continue;
            }
            n = declaration(type);
            vec_push(node->stmts, n);
            continue;
        }

        n = stmt();
        if (n->kind == ND_VAR) {
            // ローカル変数の宣言はコンパイルしない
            n = new_node(ND_NULL);
        }
        vec_push(node->stmts, n);
    }
    end_local_scope();

    return node;
}

// TODO: do~while,switch,else if
/*
 *  <stmt> = <expr>? ";"
 *         | "return" <expr>? ";"
 *         | "if" "(" <expr> ")" <stmt> ("else" <stmt>)?
 *         | "while" "(" <expr> ")" <stmt>
 *         | "for" "(" <expr>? ";" <expr>? ";" <expr>? ")" <stmt>
 *         | "for" "(" <declaration> <expr>? ";" <expr>? ")" <stmt>
 *         | "do" <stmt> "while" "(" <expr> ")" ";"
 *         | "switch" "(" <expression> ")" <statement>
 *         | ("continue" | "break") ";"
 *         | <compound_stmt>
 *         | <labeled>
 */
static Node *stmt() {
    Node *node;

    if (consume(TK_RETURN)) {
        node = new_node(ND_RETURN);
        if (consume(';')) {
            // 何も返さない場合
            node->lhs = new_node_num(0);  // ダミーで数値ノードを生成。codegenでvoid型かどうかを使って分岐
        } else {
            node->lhs = expr();
            add_type(node->lhs);
            if (!can_type_cast(node->lhs->type, cur_parse_func->ret_type->kind)) {
                error("stmt() failure: can_type_cast fail");
            }

            if (cur_parse_func->ret_type->kind == TYPE_VOID) {
                node->lhs = new_node_num(0);  // ダミーで数値ノードを生成。codegenでvoid型かどうかを使って分岐
            } else {
                node->lhs = new_cast(node->lhs, cur_parse_func->ret_type);
            }
            expect(';');
        }
    } else if (consume(TK_IF)) {
        node = new_node(ND_IF);
        expect('(');
        node->cond = expr();
        expect(')');
        node->then = stmt();
        if (consume(TK_ELSE)) {
            node->els = stmt();
        }
    } else if (consume(TK_WHILE)) {
        node = new_node(ND_WHILE);
        expect('(');
        node->cond = expr();
        expect(')');
        node->body = stmt();
    } else if (consume(TK_DO)) {
        node = new_node(ND_DO_WHILE);
        node->lhs = new_node(ND_WHILE);
        node->lhs->body = stmt();
        expect(TK_WHILE);
        expect('(');
        node->lhs->cond = expr();
        expect(')');
        expect(';');
    } else if (consume(TK_FOR)) {
        start_local_scope();
        node = new_node(ND_FOR);
        expect('(');
        // init
        if (consume_is_type_nostep(token)) {
            // <declaration>
            Type *type = declaration_specifier();
            if ((type->kind == TYPE_STRUCT || type->kind == TYPE_UNION || type->kind == TYPE_ENUM) &&
                consume_nostep('{')) {
                // 構造体か列挙型の作成
                error("stmt() failure: failure");
            }
            node->init = declaration(type);
        } else {
            // <expr>?
            if (!consume(';')) {
                node->init = expr();
                expect(';');
            }
        }
        // cond
        if (!consume(';')) {
            node->cond = expr();
            expect(';');
        }
        // inc
        if (!consume(')')) {
            node->inc = expr();
            expect(')');
        }
        node->body = stmt();
        end_local_scope();
    } else if (consume(TK_SWITCH)) {
        Node *tmp = node_in_switch;
        node = new_node(ND_SWITCH);
        node->stmts = new_vec();
        expect('(');
        node->cond = expr();
        expect(')');
        node_in_switch = node;
        node->body = stmt();
        node_in_switch = tmp;
    } else if (consume_nostep('{')) {
        node = compound_stmt();
    } else if (consume(TK_BREAK)) {
        node = new_node(ND_BREAK);
        expect(';');
    } else if (consume(TK_CONTINUE)) {
        node = new_node(ND_CONTINUE);
        expect(';');
    } else if (consume(';')) {
        node = new_node(ND_BLOCK);
        node->stmts = new_vec();
    } else if (consume_nostep(TK_CASE) || consume_nostep(TK_DEFAULT)) {
        node = labeld();
    } else {
        node = expr();
        expect(';');
    }

    return node;
}

/*
 * <labeled> = "case" <const_expr> ":" <statement>
 *           | "default" ":" <statement>
 */
static Node *labeld() {
    if (consume(TK_CASE)) {
        if (node_in_switch == NULL) {
            error("labeld() failure: switch文の中でcaseが宣言されていません");
        }
        switch_label_cnt++;

        Node *node = new_node(ND_CASE);
        node->val = const_expr();
        expect(':');

        char *name = memory_alloc(30);
        sprintf(name, ".Lswitchlabel%04d", switch_label_cnt);
        node->label_name = name;
        node->body = stmt();
        vec_push(node_in_switch->stmts, node);
        return node;
    } else if (consume(TK_DEFAULT)) {
        if (node_in_switch == NULL) {
            error("labeld() failure: switch文の中でdefaultが宣言されていません");
        }
        switch_label_cnt++;
        Node *node = new_node(ND_DEFAULT);
        expect(':');

        char *name = memory_alloc(30);
        sprintf(name, ".Lswitchlabel%04d", switch_label_cnt);
        node->label_name = name;
        node->body = stmt();
        vec_push(node_in_switch->stmts, node);
        return node;
    }

    error("labeld() failure: 不正なトークンです");
}

/*
 *  <expr> = <assign> ("," <assign>)*
 */
static Node *expr() {
    Node *node = new_node(ND_SUGER), *n = assign();
    node->stmts = new_vec();
    vec_push(node->stmts, n);
    while (consume(',')) {
        n = assign();
        vec_push(node->stmts, n);
    }

    return node;
}

// TODO: ?:, <<=, >>=, &=, ^=, |=
/*
 *  <assign> = <conditional> ("=" <assign>)?
 *           | <conditional> ( "+=" | "-=" | "*=" | "/=" | "%=" ) <conditional>
 */
static Node *assign() {
    Node *node = conditional();
    if (consume('=')) {
        node = new_assign(node, assign());
    } else if (consume(TK_ADD_EQ)) {
        node = new_assign(node, new_add(node, conditional()));
    } else if (consume(TK_SUB_EQ)) {
        node = new_assign(node, new_sub(node, conditional()));
    } else if (consume(TK_MUL_EQ)) {
        node = new_assign(node, new_mul(node, conditional()));
    } else if (consume(TK_DIV_EQ)) {
        node = new_assign(node, new_div(node, conditional()));
    } else if (consume(TK_MOD_EQ)) {
        node = new_assign(node, new_mod(node, conditional()));
    } else if (consume(TK_AND_EQ)) {
        node = new_assign(node, new_binop(ND_AND, node, conditional()));
    } else if (consume(TK_OR_EQ)) {
        node = new_assign(node, new_binop(ND_OR, node, conditional()));
    } else if (consume(TK_XOR_EQ)) {
        node = new_assign(node, new_binop(ND_XOR, node, conditional()));
    } else if (consume(TK_LSHIFT_EQ)) {
        node = new_assign(node, new_binop(ND_LSHIFT, node, conditional()));
    } else if (consume(TK_RSHIFT_EQ)) {
        node = new_assign(node, new_binop(ND_RSHIFT, node, conditional()));
    }
    return node;
}

/*
 * <conditional> = <logical_or> | <logical_or> "?" <assign> ":" <conditional>
 */
static Node *conditional() {
    Node *node = logical_or();
    if (consume('?')) {
        Node *n = new_node(ND_TERNARY);
        n->cond = node;
        n->then = assign();
        add_type(n->then);
        expect(':');
        n->els = conditional();
        add_type(n->els);
        node = n;
    }
    return node;
}

/*
 *  <logical_or> = <logical_and> ("||" <logical_and>)*
 */
static Node *logical_or() {
    Node *node = logical_and();

    for (;;) {
        if (consume(TK_LOGICAL_OR)) {
            node = new_binop(ND_LOGICAL_OR, node, logical_and());
        } else {
            return node;
        }
    }
}

/*
 *  <logical_and> = <inclusive_or> ("&&" <inclusive_or>)*
 */
static Node *logical_and() {
    Node *node = inclusive_or();

    for (;;) {
        if (consume(TK_LOGICAL_AND)) {
            node = new_binop(ND_LOGICAL_AND, node, inclusive_or());
        } else {
            return node;
        }
    }
}

/*
 * <inclusive_or> = <exclusive_or> ( "|" <exclusive_or> )*
 */
static Node *inclusive_or() {
    Node *node = exclusive_or();

    for (;;) {
        if (consume('|')) {
            node = new_binop(ND_OR, node, exclusive_or());
        } else {
            return node;
        }
    }
}

/*
 * <exclusive_or> = <bin_and> ( "^" <bin_and> )*
 */
static Node *exclusive_or() {
    Node *node = bin_and();

    for (;;) {
        if (consume('^')) {
            node = new_binop(ND_XOR, node, bin_and());
        } else {
            return node;
        }
    }
}

/*
 * <bin_and> = <equality> ( "&" <equality> )*
 */
static Node *bin_and() {
    Node *node = equality();

    for (;;) {
        if (consume('&')) {
            node = new_binop(ND_AND, node, equality());
        } else {
            return node;
        }
    }
}

/*
 *  <equality> = <relational> ("==" <relational> | "!=" <relational>)*
 */
static Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume(TK_EQ)) {
            node = new_binop(ND_EQ, node, relational());
        } else if (consume(TK_NE)) {
            node = new_binop(ND_NE, node, relational());
        } else {
            return node;
        }
    }
}

/*
 *  <relational> = <shift> ("<" <shift> | "<=" <shift> | ">" <shift> | ">=" <shift>)*
 */
static Node *relational() {
    Node *node = shift();

    for (;;) {
        if (consume('<')) {
            node = new_binop(ND_LT, node, shift());
        } else if (consume(TK_LE)) {
            node = new_binop(ND_LE, node, shift());
        } else if (consume('>')) {
            node = new_binop(ND_LT, shift(), node);
        } else if (consume(TK_GE)) {
            node = new_binop(ND_LE, shift(), node);
        } else {
            return node;
        }
    }
}

/*
 *  <shift> = <add> (">>" <add> | "<<" <add>)*
 */
static Node *shift() {
    Node *node = add();

    for (;;) {
        if (consume(TK_RSHIFT)) {
            node = new_binop(ND_RSHIFT, node, add());
        } else if (consume(TK_LSHIFT)) {
            node = new_binop(ND_LSHIFT, node, add());
        } else {
            return node;
        }
    }
}
/*
 *  <add> = <mul> ("+" <mul> | "-" <mul>)*
 */
static Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume('+')) {
            node = new_add(node, mul());
        } else if (consume('-')) {
            node = new_sub(node, mul());
        } else {
            return node;
        }
    }
}

/*
 *  <mul> = <cast> ("*" <cast> | "/" <cast> | "%" <cast> )*
 */
static Node *mul() {
    Node *node = cast();

    for (;;) {
        if (consume('*')) {
            node = new_mul(node, cast());
        } else if (consume('/')) {
            node = new_div(node, cast());
        } else if (consume('%')) {
            node = new_mod(node, cast());
        } else {
            return node;
        }
    }
}

/*
 * <cast> = "(" <type_name> ")" <cast>
 *        | <unary>
 */
static Node *cast() {
    Token *t1 = get_nafter_token(1);
    if (consume_nostep('(') && consume_is_type_nostep(t1)) {
        expect('(');
        Type *type = type_name();
        expect(')');
        Node *node = cast();
        add_type(node);
        if (can_type_cast(node->type, type->kind)) {
            node = new_cast(node, type);
        } else {
            error("can_type_cast() failure: 型のキャストに失敗しました");
        }

        return node;
    }

    return unary();
}

/*
 *  <unary> = <postfix>
 *          | "sizeof" <unary>
 *          | "sizeof" "(" <type_name> ")"
 *          | ("++" | "--") <postfix>
 *          | <unary> ("++" | "--")
 ?          | ("!" | "~" | "+" | "-" | "*" | "&") <cast>
 */
static Node *unary() {
    if (consume('+')) {
        return cast();
    } else if (consume('-')) {
        return new_sub(new_node_num(0), cast());
    } else if (consume('*')) {
        Node *node = new_node(ND_DEREF);
        node->lhs = cast();
        add_type(node->lhs);
        add_type(node);
        return node;
    } else if (consume('&')) {
        Node *node = new_node(ND_ADDR);
        node->lhs = cast();
        add_type(node->lhs);
        return node;
    } else if (consume('!')) {
        Node *node = new_node(ND_LOGICAL_NOT);
        node->lhs = cast();
        add_type(node->lhs);
        return node;
    } else if (consume('~')) {
        Node *node = new_node(ND_NOT);
        node->lhs = cast();
        add_type(node->lhs);
        return node;
    } else if (consume(TK_SIZEOF)) {
        Token *tok = get_nafter_token(1);
        if (consume_is_type_nostep(tok)) {
            expect('(');
            Type *t = type_name();
            Node *node = new_node_num(sizeOfType(t));
            expect(')');
            return node;
        } else {
            return new_node_num(sizeOfNode(unary()));
        }
    } else if (consume(TK_INC)) {
        Node *node = unary();
        return new_assign(node, new_add(node, new_node_num(1)));
    } else if (consume(TK_DEC)) {
        Node *node = unary();
        return new_assign(node, new_sub(node, new_node_num(1)));
    }

    Node *node = postfix();
    if (consume(TK_INC)) {
        // 先に+1して保存してから-1する
        return new_sub(new_assign(node, new_add(node, new_node_num(1))), new_node_num(1));
    } else if (consume(TK_DEC)) {
        // 先に+-1して保存してから+1する
        return new_add(new_assign(node, new_sub(node, new_node_num(1))), new_node_num(1));
    }

    return node;
}

/*
 *  <postfix> = <primary>  ( ("[" <expr> "]") | "." | "->" ) *
 */
static Node *postfix() {
    Node *node = primary();

    while (1) {
        if (consume_nostep(TK_ARROW) || consume_nostep('.')) {
            if (consume_nostep(TK_ARROW)) {
                Node *n = new_node(ND_DEREF);
                n->lhs = node;
                add_type(n->lhs);
                add_type(n);
                node = n;
            }
            next_token();
            // 構造体のメンバーアクセス
            Token *tok = token;
            expect(TK_IDENT);
            add_type(node);
            if (node->type->kind != TYPE_STRUCT && node->type->kind != TYPE_UNION) {
                error("postfix() failure: struct or union型ではありません。");
            }
            Var *member = node->type->member;
            while (member) {
                if (!strcmp(member->name, my_strndup(tok->str, tok->len))) {
                    Node *n = new_node(ND_STRUCT_MEMBER);
                    // 変数をコピー
                    n->lhs = node;
                    n->val = member->offset;
                    n->type = member->type;
                    node = n;
                    break;
                }
                member = member->next;
            }

            if (member == NULL) {
                error("postfix() failure: %s構造体が定義されていません。", my_strndup(tok->str, tok->len));
            }

            continue;
        }

        if (consume('[')) {
            Node *deref = new_node(ND_DEREF);
            deref->lhs = new_add(node, expr());
            add_type(deref->lhs);
            add_type(deref);
            expect(']');
            node = deref;
            continue;
        }

        break;
    }

    return node;
}

/*
 *  <funcall> = "(" (<assign> ("," <assign>)*)? ")"
 */
static Node *funcall(Token *tok) {
    expect('(');
    Node *node = new_node(ND_CALL);

    node->args = new_vec();
    while (!consume(')')) {
        if (node->args->len != 0) {
            expect(',');
        }
        Node *n = assign();
        add_type(n);
        vec_push(node->args, n);
    }

    char *name = my_strndup(tok->str, tok->len);
    Function *fn = find_func(name);

    if (fn) {
        /* 定義された関数 */
        node->fn_name = name;
        should_cast_args(node->args, fn->params, fn->is_variadic);
    } else {
        /* 関数ポインター */
        Var *v = find_allscope_var(tok);
        if (v) {
            node->fn_name = "";
            node->lhs = should_new_node_var(tok);
        } else {
            error("funcalL() failure: %sは定義されていない関数・変数です", name);
        }
    }

    if (strcmp(node->fn_name, "va_start") == 0) {
        /*
         * va_startをマクロとして実装できないので、内部で va_start(ap, fmt)を
         * *ap = *(struct __builtin_va_list *)__va_area__
         * に置換する。
         */

        // 仮引数のサイズ
        if (node->args->len != 2) {
            error("funcall() failure: va_start args len != 2");
        }

        // __builtin_va_list構造体が定義されているか
        Tag *tag = find_gstruct_type("__builtin_va_list");
        if (tag == NULL) {
            error("find_gstruct_type() failure: __builtin_va_listが未定義です");
        }
        // struct __builtin_va_list *
        Type *t = new_ptr_type(tag->base_type);

        // lhs -> *ap
        Node *lhs = new_node(ND_DEREF);
        lhs->lhs = node->args->body[0];
        add_type(lhs->lhs);
        add_type(lhs);

        // rhs -> *(struct __builtin_va_list *)__va_area__
        Token *tok = memory_alloc(sizeof(Token));
        tok->str = "__va_area__";
        tok->len = strlen(tok->str);

        Node *rhs_ident = should_new_node_var(tok);

        Node *rhs = new_node(ND_DEREF);
        rhs->lhs = new_node(ND_CAST);
        rhs->lhs->lhs = rhs_ident;
        rhs->lhs->type = t;
        node = new_assign(lhs, rhs);
    }
    return node;
}

/*
 *  <primary> = "(" <expr> ")" | "(" <compound_stmt> ")" | <num> | <string> | <ident> <funcall>?
 */
static Node *primary() {
    if (consume('(')) {
        if (consume_nostep('{')) {
            Node *node = compound_stmt();
            // BLOCKをSTMT_EXPRに書き換える
            node->kind = ND_STMT_EXPR;
            expect(')');
            return node;
        } else {
            Node *node = expr();
            expect(')');
            return node;
        }
    }

    if (consume_nostep(TK_IDENT)) {
        Token *tok = token;
        next_token();
        Node *node;

        // enum
        Var *v = find_enum_member(tok);
        if (v) {
            return new_node_num(v->val);
        }

        if (consume_nostep('(')) {
            node = funcall(tok);
        } else {
            node = should_new_node_var(tok);
        }
        return node;
    }

    if (consume_nostep(TK_STRING)) {
        Node *node = new_node(ND_STRING);
        node->str_literal = token->str;
        node->val = token->str_literal_index;
        next_token();
        return node;
    }

    if (consume_nostep(TK_NUM)) {
        Token *t = token;
        Node *node = new_node_num(expect_number());
        node->type = t->type;
        return node;
    }

    if (token->kind == TK_EOF) {
        // TK_EOFはtoken->strが入力を超える位置になる
        error("primary() failure: 不正なコードです。");
    }
    error_at(token->str, "primary() failure: 不正なトークンです。");
}

static long const_expr() {
    GInitEl *g = eval(expr());
    if (g->len > 0) {
        error("const_expr() failure: 定数ではありません");
    }
    return g->val;
}
