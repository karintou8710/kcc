#include "kcc.h"

/*
 * ローカル変数を単方向の連結リストで持つ
 * localsは後に出たローカル変数のポインタを持つ
 */
static Var *locals;
static Function *cur_parse_func;
static Vector *local_scope;
static bool is_global = true;
static StorageClass current_storage = UNKNOWN;

static Type *find_typedef_alias(char *name);

/* nodeの生成 */
static Node *new_add(Node *lhs, Node *rhs);
static Node *new_sub(Node *lhs, Node *rhs);
static Node *new_mul(Node *lhs, Node *rhs);
static Node *new_div(Node *lhs, Node *rhs);
static Node *new_mod(Node *lhs, Node *rhs);
static Node *new_node_num(long val);
static Var *new_lvar(Token *tok, Type *type);
static Var *new_gvar(Token *tok, Type *type);
static void create_lvar_from_params(Var *params);
static Var *find_lvar(Token *tok);
static Var *find_gvar(Token *tok);
static Vector *new_node_init2(Initializer *init, Node *node);

/* AST */
static Type *type_specifier();
static Var *enumerator(Type *type, int *enum_const_num);
static void enumerator_list(Type *type);
static void initialize2(Initializer *init);
static Node *declaration_global(Type *type);
static Node *declaration_var(Type *type);
static Node *declaration(Type *type);
static Var *declaration_param(Var *cur);
static Type *pointer(Type *type);
static Function *func_define();
static Node *compound_stmt();
static Node *stmt();
static Node *expr();
static Node *assign();
static Node *conditional();
static Node *logical_expression();
static Node *inclusive_or();
static Node *exclusive_or();
static Node * and ();
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
static GInit_el *eval(Node *node);

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
    if (tok->kind == TK_TYPE) {
        return true;
    }

    if (tok->kind == TK_IDENT) {
        char *name = my_strndup(tok->str, tok->len);
        return (bool)find_typedef_alias(name);
    }

    // 基礎型でもtypedefでもない
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

/* ノードを引数にもつsizeofの実装 */
static int sizeOfNode(Node *node) {
    add_type(node);
    return sizeOfType(node->type);
}

/* ローカル変数の作成 */
static Var *new_lvar(Token *tok, Type *type) {
    Var *lvar = memory_alloc(sizeof(Var));
    lvar->next = locals;
    lvar->name = my_strndup(tok->str, tok->len);
    lvar->len = tok->len;
    lvar->type = type;
    if (locals->next_offset > 0) {
        lvar->offset = locals->next_offset + sizeOfType(type);
    } else {
        lvar->offset = locals->offset + sizeOfType(type);
    }

    locals = lvar;  // localsを新しいローカル変数に更新
    return lvar;
}

static Var *new_gvar(Token *tok, Type *type) {
    Var *gvar = memory_alloc(sizeof(Var));
    gvar->next = globals;
    gvar->name = my_strndup(tok->str, tok->len);
    gvar->len = tok->len;
    gvar->type = type;
    gvar->is_global = true;
    gvar->ginit = new_vec();
    globals = gvar;  // globalsを新しいグローバル変数に更新
    return gvar;
}

static void new_struct_member(Token *tok, Type *member_type, Type *struct_type) {
    Var *member = memory_alloc(sizeof(Var));
    member->next = struct_type->member;
    member->name = my_strndup(tok->str, tok->len);
    member->len = tok->len;
    member->type = member_type;
    member->offset = struct_type->member->offset + sizeOfType(member_type);
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

static Typedef_alias *new_typedef_alias(char *name, Type *type) {
    Typedef_alias *ta = memory_alloc(sizeof(Typedef_alias));
    ta->name = name;
    ta->type = type;
    return ta;
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

/* 既に定義されたローカル変数を検索 */
static Var *find_lvar(Token *tok) {
    Var *vars = locals;
    for (Var *var = vars; var; var = var->next) {
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
            return var;
        }
    }
    return NULL;
}

/* 既に定義されたローカル変数を検索 */
static Var *find_scope_lvar(Token *tok) {
    Var *vars = locals, *scope = vec_last(local_scope);
    for (Var *var = vars; var != scope; var = var->next) {
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
            return var;
        }
    }
    return NULL;
}

/* 既に定義されたグローバル変数を検索 */
static Var *find_gvar(Token *tok) {
    Var *vars = globals;
    for (Var *var = vars; var; var = var->next) {
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
            return var;
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

bool is_already_defined_global_obj(Token *tok) {
    char *name = my_strndup(tok->str, tok->len);
    return find_gvar(tok) || find_func(name);
}

bool is_same_params(Var *params1, Var *params2) {
    for (Var *v1 = params1, *v2 = params2;; v1 = v1->next, v2 = v2->next) {
        if (v1 == NULL || v2 == NULL) {
            // NULL == NULL -> params1とparams2は等しい
            return v1 == v2;
        }

        if (!is_same_type(params1->type, params2->type)) {
            return false;
        }
    }
}

bool has_lvar_in_all_params(Var *params) {
    for (Var *v = params; v; v = v->next) {
        if (v->is_only_type) return false;
    }
    return true;
}

Var *find_params(char *name, Var *params) {
    if (name == NULL) return NULL;
    Var *vars = params;
    for (Var *var = vars; var; var = var->next) {
        if (!var->name) continue;
        if (strcmp(name, var->name) == 0) {
            return var;
        }
    }
    return NULL;
}

static Type *find_lstruct_type(char *name) {
    if (name == NULL) return NULL;
    for (int i = 0; i < struct_local_lists->len; i++) {
        Type *t = struct_local_lists->body[i];
        if (t->name == NULL) continue;
        if (strcmp(t->name, name) == 0) {
            return t;
        }
    }

    return NULL;
}

static Type *find_gstruct_type(char *name) {
    if (name == NULL) return NULL;
    for (int i = 0; i < struct_global_lists->len; i++) {
        Type *t = struct_global_lists->body[i];
        if (t->name == NULL) continue;
        if (strcmp(t->name, name) == 0) {
            return t;
        }
    }

    return NULL;
}

// enum型を探索
static Type *find_lenum_type(char *name) {
    if (name == NULL) return NULL;
    for (int i = 0; i < enum_local_lists->len; i++) {
        Type *t = enum_local_lists->body[i];
        if (t->name == NULL) continue;
        if (strcmp(t->name, name) == 0) {
            return t;
        }
    }

    return NULL;
}

static Type *find_genum_type(char *name) {
    if (name == NULL) return NULL;
    for (int i = 0; i < enum_global_lists->len; i++) {
        Type *t = enum_global_lists->body[i];
        if (t->name == NULL) continue;
        if (strcmp(t->name, name) == 0) {
            return t;
        }
    }

    return NULL;
}

static Type *find_enum_type(char *name) {
    // ローカルで探索
    Type *t = find_lenum_type(name);
    if (t) return t;

    // グローバルで探索
    t = find_genum_type(name);
    return t;
}

// enum型のメンバーを探索
static Var *find_lenum_member(char *name) {
    if (name == NULL) return NULL;
    for (int i = 0; i < enum_local_lists->len; i++) {
        Type *t = enum_local_lists->body[i];
        for (Var *v = t->member; v; v = v->next) {
            if (!v->name) continue;
            if (strcmp(v->name, name) == 0) {
                return v;
            }
        }
    }

    return NULL;
}

static Var *find_genum_member(char *name) {
    if (name == NULL) return NULL;
    for (int i = 0; i < enum_global_lists->len; i++) {
        Type *t = enum_global_lists->body[i];
        for (Var *v = t->member; v; v = v->next) {
            if (!v->name) continue;
            if (strcmp(v->name, name) == 0) {
                return v;
            }
        }
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

static Type *is_defined_enum_type(char *name) {
    if (is_global) {
        Type *t = find_genum_type(name);
        if (t != NULL && !t->is_forward) {
            error("find_genum_type() failure: %s列挙型は既に宣言済みです。", name);
        }
        // 前方宣言
        return t;
    } else {
        Type *t = find_lenum_type(name);
        if (t != NULL && !t->is_forward) {
            error("find_lenum_type() failure: %s列挙型は既に宣言済みです。", name);
        }
        // 前方宣言
        return t;
    }
}

static Type *find_typedef_alias(char *name) {
    if (name == NULL) return NULL;
    for (int i = 0; i < typedef_alias->len; i++) {
        Typedef_alias *ta = typedef_alias->body[i];
        if (!ta->name) continue;
        if (strcmp(ta->name, name) == 0) {
            return ta->type;
        }
    }

    return NULL;
}

static void start_local_scope() {
    vec_push(local_scope, locals);
}

static void end_local_scope() {
    Var *var = vec_pop(local_scope);
    var->next_offset = locals->next_offset > 0 ? locals->next_offset : locals->offset;
    locals = var;
}

static Initializer *new_initializer(Var *var) {
    Initializer *init = memory_alloc(sizeof(Initializer));
    init->type = var->type;
    init->var = var;
    return init;
}

static void eval_concat(GInit_el *g, GInit_el *gl, GInit_el *gr, char *op, int len) {
    int max_digit = 50;
    if (gl->str && gr->str) {
        error("eval_concat() failure: オペランドが不適切です [%s]", op);
    } else if (gl->str && !gr->str) {
        if (strcmp(op, "+") != 0 && strcmp(op, "-") != 0) {
            error("eval_concat() failure: オペランドが不適切です [%s]", op);
        }
        int len = gl->len + max_digit + len + 1;
        char *buf = memory_alloc(sizeof(char) * len);
        len = snprintf(buf, len, "%s %s %ld", gl->str, op, gr->val);
        g->str = buf;
        g->len = len;
    } else if (!gl->str && gr->str) {
        if (strcmp(op, "+") != 0 && strcmp(op, "-") != 0) {
            error("eval_concat() failure: オペランドが不適切です [%s]", op);
        }
        int len = max_digit + gr->len + len + 1;
        char *buf = memory_alloc(sizeof(char) * len);
        len = snprintf(buf, len, "%ld %s %s", gl->val, op, gr->str);
        g->str = buf;
        g->len = len;
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
        }
    }
}

/* TODO: 四則演算以外にも対応 */
static GInit_el *eval(Node *node) {
    GInit_el *g = memory_alloc(sizeof(GInit_el));
    add_type(node);

    if (node->kind == ND_NUM) {
        g->val = node->val;
        return g;
    } else if (node->kind == ND_ADD) {
        GInit_el *gl = eval(node->lhs);
        GInit_el *gr = eval(node->rhs);
        eval_concat(g, gl, gr, "+", 2);
        return g;
    } else if (node->kind == ND_SUB) {
        GInit_el *gl = eval(node->lhs);
        GInit_el *gr = eval(node->rhs);
        g->val = gl->val - gr->val;
        eval_concat(g, gl, gr, "-", 2);
        return g;
    } else if (node->kind == ND_MUL) {
        GInit_el *gl = eval(node->lhs);
        GInit_el *gr = eval(node->rhs);
        eval_concat(g, gl, gr, "*", 2);
        return g;
    } else if (node->kind == ND_DIV) {
        GInit_el *gl = eval(node->lhs);
        GInit_el *gr = eval(node->rhs);
        eval_concat(g, gl, gr, "/", 2);
        return g;
    } else if (node->kind == ND_MOD) {
        GInit_el *gl = eval(node->lhs);
        GInit_el *gr = eval(node->rhs);
        eval_concat(g, gl, gr, "%", 2);
        return g;
    } else if (node->kind == ND_EQ) {
        GInit_el *gl = eval(node->lhs);
        GInit_el *gr = eval(node->rhs);
        eval_concat(g, gl, gr, "==", 3);
        return g;
    } else if (node->kind == ND_NE) {
        GInit_el *gl = eval(node->lhs);
        GInit_el *gr = eval(node->rhs);
        eval_concat(g, gl, gr, "!=", 3);
        return g;
    } else if (node->kind == ND_LT) {
        GInit_el *gl = eval(node->lhs);
        GInit_el *gr = eval(node->rhs);
        eval_concat(g, gl, gr, "<", 2);
        return g;
    } else if (node->kind == ND_LE) {
        GInit_el *gl = eval(node->lhs);
        GInit_el *gr = eval(node->rhs);
        eval_concat(g, gl, gr, "<=", 3);
        return g;
    } else if (node->kind == ND_LOGICALNOT) {
        GInit_el *gl = eval(node->lhs);
        GInit_el *gr = memory_alloc(sizeof(GInit_el));
        eval_concat(g, gl, gr, "!", 2);
        return g;
    } else if (node->kind == ND_LOGICAL_AND) {
        GInit_el *gl = eval(node->lhs);
        GInit_el *gr = eval(node->rhs);
        eval_concat(g, gl, gr, "&&", 3);
        return g;
    } else if (node->kind == ND_LOGICAL_OR) {
        GInit_el *gl = eval(node->lhs);
        GInit_el *gr = eval(node->rhs);
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
    }

    error("未対応のNodeタイプです");
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

    error("new_add() failure: 実行できない型による演算です");
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

    Node *node = new_binop(ND_ASSIGN, lhs, rhs);
    // 代入できるかチェック
    add_type(node);

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
        for (int i = 0; i < init->len; i++) {
            Node *deref = new_node(ND_DEREF);
            deref->lhs = new_add(node, new_node_num(i));
            add_type(deref->lhs);
            add_type(deref);
            Vector *v = new_node_init2(init->children + i, deref);
            vec_concat(suger, v);
        }
        return suger;
    }

    if (is_global) {
        vec_push(init->var->ginit, eval(init->expr));
    } else {
        vec_push(suger, new_assign(node, init->expr));
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
static Node *get_node_ident(Token *tok) {
    Node *node = new_node(ND_VAR);
    Var *var;
    var = find_lvar(tok);  // ローカル変数を取得
    if (!var) {
        var = find_gvar(tok);  // グローバル変数から取得
        if (!var) {
            error_at(tok->str, "get_node_ident() failure: 宣言されていません");
        }
    }

    node->var = var;
    return node;
}

/*************************************/
/******                         ******/
/******           AST           ******/
/******                         ******/
/*************************************/

/*
 *  <program> = ( <declaration_global> | <func_define> )*
 */
void program() {
    int i = 0;
    local_scope = new_vec();
    while (!at_eof()) {
        Type *type = type_specifier();
        if (is_func(token)) {
            is_global = false;
            Function *fn = func_define(type);
            if (fn != NULL) vec_push(funcs, fn);
            is_global = true;
        } else {
            Node *node = declaration_global(type);
        }
    }
}

/*
 *  <declaration_global> = <declaration> ";"
 */
static Node *declaration_global(Type *type) {
    if ((type->kind == TYPE_STRUCT || type->kind == TYPE_ENUM) &&
        consume(';')) {
        // 構造体・列強型の作成 or 宣言
        return new_node(ND_NULL);
    }
    Node *node = declaration(type);
    expect(';');
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
        int children_cap = 2;
        init->children = memory_alloc(sizeof(Initializer) * children_cap);
        expect('{');
        int i = 0;
        while (!consume('}')) {
            if (i >= children_cap) {
                children_cap *= 2;
                init->children = realloc(init->children, sizeof(Initializer) * children_cap);
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

        expect('{');
        for (int i = 0; i < init->len; i++) {
            (init->children + i)->var = init->var;
            if (consume_nostep('}')) {
                (init->children + i)->expr = new_node_num(0);
                continue;
            }

            if (i > 0) expect(',');
            (init->children + i)->type = ty->ptr_to;
            initialize2(init->children + i);
        }
        expect('}');
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
        if (token->len <= i) {
            (init->children + i)->expr = new_node_num(0);
            continue;
        }
        (init->children + i)->type = ty->ptr_to;
        (init->children + i)->expr = new_node_num(token->str[i]);
    }
    next_token();
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

    init->expr = assign();
}

/*
 *  <pointer> = "*"*
 */
static Type *pointer(Type *type) {
    while (consume('*')) {
        Type *t = new_ptr_type(type);
        type = t;
    }
    return type;
}

/*
 *  <declaration_var> = <pointer> <ident> <type_suffix> ("=" <initialize>)?
 */
static Node *declaration_var(Type *type) {
    type = pointer(type);
    Node *node = declear_node_ident(token, type);
    next_token();

    if (consume_nostep('[')) {
        // 配列
        node->var->type = type_suffix(node->var->type, true);
        // 新しい型のオフセットにする
        node->var->offset += sizeOfType(node->var->type) - sizeOfType(type);
    }
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
 *  <declaration> = <type_specifier> <declaration_var> ("," <declaration_var>)*
 */
static Node *declaration(Type *type) {
    Node *node = declaration_var(type);
    if (consume_nostep(';')) {
        if (node->kind == ND_VAR && current_storage == STORAGE_TYPEDEF) {
            Typedef_alias *ta = new_typedef_alias(node->var->name, node->var->type);
            vec_push(typedef_alias, ta);
            current_storage = UNKNOWN;
        } else if (node->kind == ND_VAR && current_storage == STORAGE_EXTERN) {
            node->var->is_extern = true;
            current_storage = UNKNOWN;
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
            Typedef_alias *ta = new_typedef_alias(tmp_node->var->name, tmp_node->var->type);
            vec_push(typedef_alias, ta);
        } else if (tmp_node->kind == ND_VAR && current_storage == STORAGE_EXTERN) {
            tmp_node->var->is_extern = true;
        }
    }
    current_storage = UNKNOWN;

    return n;
}

/*
 *  <struct_declaration> = <type_specifier> <pointer> <ident> ";"
 */
Type *struct_declaration(Type *type) {
    Type *t = type_specifier();
    t = pointer(t);
    Token *tok = token;
    next_token();
    t = type_suffix(t, false);
    new_struct_member(tok, t, type);
    expect(';');
    return type;
}

/*
 *  <storage_class>  = "typedef" | "extern"
 *  <type_specifier> = <storage_class>? "int"
 *                   | <storage_class>? "char"
 *                   | <storage_class>? "void"
 *                   | <storage_class>? "struct" <ident>
 *                   | <storage_class>? "struct" <ident> "{" <struct_declaration>* "}"
 */
static Type *type_specifier() {
    if (consume(TK_TYPEDEF))
        current_storage = STORAGE_TYPEDEF;
    else if (consume(TK_EXTERN)) {
        current_storage = STORAGE_EXTERN;
    }

    Token *tok = token;
    Type *type;

    if (consume(TK_IDENT)) {
        // typedef
        char *name = my_strndup(tok->str, tok->len);
        type = find_typedef_alias(name);
        if (type == NULL) {
            error("find_typedef_alias() failure: %sは定義されていません", name);
        }
    } else if (consume(TK_TYPE)) {
        // 基礎型
        type = tok->type;
    } else {
        error("type_specifier() failure: 適切な型ではありません");
    }

    if (type->kind == TYPE_STRUCT && consume('{')) {
        if (is_global) {
            Type *t = find_gstruct_type(type->name);
            if (t != NULL && !t->is_forward) {
                error("find_gstruct_type() failure: %s構造体は既に宣言済みです。", type->name);
            }
            if (t) type = t;
        } else {
            Type *t = find_lstruct_type(type->name);
            if (t != NULL && !t->is_forward) {
                error("find_lstruct_type() failure: %s構造体は既に宣言済みです。", type->name);
            }
            if (t) type = t;
        }

        if (!type->is_forward) {
            vec_push(is_global ? struct_global_lists : struct_local_lists, type);
        } else {
            type->is_forward = false;
        }

        // 構造体のメンバーの宣言
        type->member = memory_alloc(sizeof(Var));
        while (!consume('}')) {
            type = struct_declaration(type);
        }
        // 定義した順に並べ直す
        Var *reverse_member = NULL;
        while (type->member) {
            Var *tmp = type->member->next;
            type->member->next = reverse_member;
            reverse_member = type->member;
            type->member = tmp;
        }
        type->member = reverse_member->next;
        return type;
    }

    if (type->kind == TYPE_ENUM && consume('{')) {
        // is_defined_enum_typeは列挙型が既に定義されていたら、強制終了する
        Type *t = is_defined_enum_type(type->name);
        if (t != NULL) {
            // 既に前方宣言がされている
            type = t;
        }
        enumerator_list(type);

        if (!type->is_forward) {
            vec_push(is_global ? enum_global_lists : enum_local_lists, type);
        } else {
            type->is_forward = false;
        }

        return type;
    }

    if (type->kind == TYPE_STRUCT) {
        Type *stype = find_lstruct_type(type->name);
        if (stype == NULL) {
            stype = find_gstruct_type(type->name);
            if (stype == NULL) {
                type->is_forward = true;
                stype = type;
                vec_push(is_global ? struct_global_lists : struct_local_lists, stype);
            }
        }
        type = stype;
    }

    if (type->kind == TYPE_ENUM) {
        Type *etype = find_enum_type(type->name);
        if (etype == NULL) {
            type->is_forward = true;
            etype = type;
            vec_push(is_global ? struct_global_lists : struct_local_lists, etype);
        }
        type = etype;
    }

    return type;
}

Type *type_name() {
    Type *type = type_specifier();
    type = pointer(type);
    type = type_suffix(type, true);
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
    (*enum_const_num)++;

    if (consume('=')) {
        GInit_el *el = eval(conditional());
        if (el->str) {
            error("enumerator() failure: 数値型の定数ではありません");
        }
        var->val = el->val;
        (*enum_const_num) = el->val + 1;
    }
    return var;
}

// is_firstは配列の初期化時のみ使用
/*
 *  <type_suffix> = "[" <num>? "]" <type_suffix> | ε
 */
static Type *type_suffix(Type *type, bool is_first) {
    if (consume('[')) {
        int array_size;
        if (is_first && consume(']')) {
            array_size = 0;
        } else {
            array_size = expect_number();
            expect(']');
        }
        type = new_array_type(type_suffix(type, false), array_size);
    }

    return type;
}

/*
 *  <declaration_param> = <type_specifier> <pointer> <ident> <type_suffix>
 */
static Var *declaration_param(Var *cur) {
    Type *type = type_specifier();
    type = pointer(type);
    Token *tok = token;
    Var *lvar = memory_alloc(sizeof(Var));
    lvar->type = type;
    lvar->offset = cur->offset + sizeOfType(lvar->type);
    if (consume(TK_IDENT)) {
        lvar->name = my_strndup(tok->str, tok->len);
        lvar->len = tok->len;
        lvar->is_only_type = false;
    } else {
        lvar->is_only_type = true;
    }

    if (consume_nostep('[')) {
        // ポインタとして受け取る
        // 最初の添え字を省略した配列は、ポインター型として扱うので処理の分岐は必要ない
        lvar->type = type_suffix(lvar->type, true);
        // 新しい型のオフセットにする
        lvar->offset += sizeOfType(lvar->type) - sizeOfType(type);
    }
    return lvar;
}

/*
 *  <func_define> = <type_specifier> <pointer> <ident>
 *                  "(" (<declaration_param> ("," <declaration_param>)* | "void" | ε)  ")"
 *                  <compound_stmt>
 */
static Function *func_define(Type *type) {
    type = pointer(type);
    Function *fn = memory_alloc(sizeof(Function));
    cur_parse_func = fn;
    Token *tok = token;
    Var head = {};
    Var *cur = &head;  // 引数の単方向連結リスト
    bool is_variadic = false;

    expect(TK_IDENT);
    fn->name = my_strndup(tok->str, tok->len);
    if (find_gvar(tok)) {
        error("func_define() failure: 既に%sは定義されています", fn->name);
    }
    fn->ret_type = type;
    expect('(');

    // (void)の場合
    if (token->kind == TK_TYPE &&
        token->type->kind == TYPE_VOID &&
        get_nafter_token(1)->kind == ')') {
        expect(TK_TYPE);
    }

    while (!consume(')')) {
        if (cur != &head) {
            expect(',');
        }

        if (consume(TK_VARIADIC)) {
            if (cur == &head) {
                error("func_define() failure: ...は第一引数に設定できません");
            }

            is_variadic = true;
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

    fn->params = head.next;  // 前から見ていく
    fn->is_variadic = is_variadic;

    Function *entry = find_func(fn->name);
    if (consume(';')) {
        // プロトタイプ宣言
        fn->is_prototype = true;
        if (entry == NULL) {
            return fn;
        }

        if (!is_same_params(fn->params, entry->params) ||
            !is_same_type(fn->ret_type, entry->ret_type)) {
            error("func_define() failure: 異なる型でのプロトタイプ宣言です");
        }

        return NULL;
    }

    // 定義
    if (entry) {
        if (!entry->is_prototype) {
            error("func_define() failure: 既に%sは定義されています", fn->name);
        }

        if (!is_same_params(fn->params, entry->params) ||
            !is_same_type(fn->ret_type, entry->ret_type)) {
            error("func_define() failure: 異なる型での宣言です");
        }
    }

    // 定義では型だけの引数を許容しない
    if (!has_lvar_in_all_params(fn->params)) {
        error("has_lvar_in_all_params() failure: 引数の定義には変数名が必要です");
    }

    locals = memory_alloc(sizeof(Var));
    struct_local_lists = new_vec();  // 関数毎に構造体を初期化
    enum_local_lists = new_vec();
    locals->offset = 0;
    start_local_scope();
    create_lvar_from_params(fn->params);
    fn->body = compound_stmt();
    end_local_scope();
    fn->locals = locals;
    return fn;
}

/*
 *  <compound_stmt> = { <stmt>* }
 */
static Node *compound_stmt() {
    expect('{');
    // ローカルのスコープを取る為に、現在のローカル変数を保持
    start_local_scope();
    Node *node = new_node(ND_BLOCK);
    node->stmts = new_vec();
    while (!consume('}')) {
        Node *n = stmt();
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
 *         | ("continue" | "break")
 *         | <compound_stmt>
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
    } else if (consume(TK_FOR)) {
        start_local_scope();
        node = new_node(ND_FOR);
        expect('(');
        if (!consume(';')) {
            node->init = expr();
            expect(';');
        }
        if (!consume(';')) {
            node->cond = expr();
            expect(';');
        }
        if (!consume(')')) {
            node->inc = expr();
            expect(')');
        }
        node->body = stmt();
        end_local_scope();
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
    } else {
        node = expr();
        expect(';');
    }

    return node;
}

/*
 *  <expr> = <assign> ("," <assign>)* | <declaration>
 */
static Node *expr() {
    if (consume_is_type_nostep(token)) {
        Type *type = type_specifier();
        if ((type->kind == TYPE_STRUCT || type->kind == TYPE_ENUM) &&
            consume_nostep(';')) {
            // 構造体か列挙型の作成 or 宣言
            return new_node(ND_NULL);
        }

        Node *node = declaration(type);
        return node;
    }

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
 * <conditional> = <logical_expression> | <logical_expression> "?" <assign> ":" <conditional>
 */
static Node *conditional() {
    Node *node = logical_expression();
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
 *  <logical_expression> = <equality> ("&&" <equality> | "||" <equality>)*
 */
static Node *logical_expression() {
    Node *node = inclusive_or();

    for (;;) {
        if (consume(TK_LOGICAL_AND)) {
            node = new_binop(ND_LOGICAL_AND, node, inclusive_or());
        } else if (consume(TK_LOGICAL_OR)) {
            node = new_binop(ND_LOGICAL_OR, node, inclusive_or());
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
 * <exclusive_or> = <and> ( "^" <and> )*
 */
static Node *exclusive_or() {
    Node *node = and();

    for (;;) {
        if (consume('^')) {
            node = new_binop(ND_XOR, node, and());
        } else {
            return node;
        }
    }
}

/*
 * <and> = <equality> ( "&" <equality> )*
 */
static Node * and () {
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
            Node *n = new_node(ND_CAST);
            n->lhs = node;
            n->type = type;
            node = n;
        } else {
            error("can_type_cast() failure: 型のキャストに失敗しました");
        }

        return node;
    }

    return unary();
}

/*
 *  <unary> = "+"? <postfix>
 *          | "-"? <postfix>
 *          | "*" <unary>
 *          | "&" <postfix>
 *          | "sizeof" <unary>
 *          | "sizeof" "(" <type_name> ")"
 *          | ("++" | "--") <postfix>
 *          | <postfix> ("++" | "--")
 *          | "!" <unary>
 *          | "~" <unary>
 */
static Node *unary() {
    if (consume('+')) {
        return postfix();
    } else if (consume('-')) {
        return new_sub(new_node_num(0), postfix());
    } else if (consume('*')) {
        Node *node = new_node(ND_DEREF);
        node->lhs = unary();
        add_type(node->lhs);
        add_type(node);
        return node;
    } else if (consume('&')) {
        Node *node = new_node(ND_ADDR);
        node->lhs = postfix();
        add_type(node->lhs);
        return node;
    } else if (consume('!')) {
        Node *node = new_node(ND_LOGICALNOT);
        node->lhs = unary();
        add_type(node->lhs);
        return node;
    } else if (consume('~')) {
        Node *node = new_node(ND_NOT);
        node->lhs = unary();
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
        Node *node = postfix();
        return new_assign(node, new_add(node, new_node_num(1)));
    } else if (consume(TK_DEC)) {
        Node *node = postfix();
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
            if (node->type->kind != TYPE_STRUCT) {
                error("postfix() failure: struct型ではありません。");
            }
            Var *member = node->type->member;
            while (member) {
                if (!strcmp(member->name, my_strndup(tok->str, tok->len))) {
                    Node *n = new_node(ND_STRUCT_MEMBER);
                    // 変数をコピー
                    n->lhs = node;
                    n->val = member->offset - member->type->size;
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
    node->fn_name = my_strndup(tok->str, tok->len);
    node->args = new_vec();
    while (!consume(')')) {
        if (node->args->len != 0) {
            expect(',');
        }
        vec_push(node->args, assign());
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
            node = get_node_ident(tok);
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