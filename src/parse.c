#include "9cc.h"

/*
 * ローカル変数を単方向の連結リストで持つ
 * localsは後に出たローカル変数のポインタを持つ
 */
static Var *locals;
static Function *cur_parse_func;
static Vector *local_scope;
static bool is_global = true;
static bool is_struct_create = false;

/* nodeの生成 */
static Node *new_add(Node *lhs, Node *rhs);
static Node *new_sub(Node *lhs, Node *rhs);
static Node *new_mul(Node *lhs, Node *rhs);
static Node *new_div(Node *lhs, Node *rhs);
static Node *new_mod(Node *lhs, Node *rhs);
static Node *new_node_num(int val);
static Var *new_lvar(Token *tok, Type *type);
static Var *new_gvar(Token *tok, Type *type);
static void create_lvar_from_params(Var *params);
static Var *find_lvar(Token *tok);
static Var *find_gvar(Token *tok);
static void init_initializer(Initializer *init, Type *ty);
static Vector *new_node_init2(Initializer *init, Node *node);

/* AST */
static Type *type_specifier();
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
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *postfix();
static Type *type_suffix(Type *type);
static Node *primary();

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

static int expect_number() {
    if (token->kind != TK_NUM) {
        error_at(token->str, "expect_number() failure: 数値ではありません");
    }
    int val = token->val;
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
    for (int i = 0; funcs[i]; i++) {
        if (!memcmp(name, funcs[i]->name, strlen(name))) {
            return funcs[i];
        }
    }

    return NULL;
}

static Type *find_lstruct_type(char *name) {
    for (int i = 0; i < struct_local_lists->len; i++) {
        Type *t = struct_local_lists->body[i];
        if (strcmp(t->name, name) == 0) {
            return t;
        }
    }

    return NULL;
}

static Type *find_gstruct_type(char *name) {
    for (int i = 0; i < struct_global_lists->len; i++) {
        Type *t = struct_global_lists->body[i];
        if (strcmp(t->name, name) == 0) {
            return t;
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

static Initializer *new_initializer(Type *ty) {
    Initializer *init = memory_alloc(sizeof(Initializer));
    init_initializer(init, ty);
    return init;
}

static void init_initializer(Initializer *init, Type *ty) {
    init->type = ty;

    if (ty->kind == TYPE_ARRAY) {
        init->children = memory_alloc(sizeof(Initializer) * ty->array_size);
        init->len = ty->array_size;
        for (int i = 0; i < ty->array_size; i++) {
            init_initializer(init->children + i, ty->ptr_to);
        }
        return;
    }

    return;
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
static Node *new_node_num(int val) {
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

    vec_push(suger, new_assign(node, init->expr));
    return suger;
}

/* 変数を宣言 */
static Node *declear_node_ident(Token *tok, Type *type) {
    Node *node = new_node(ND_VAR);
    Var *var = is_global ? find_gvar(tok) : find_scope_lvar(tok);
    if (var) {
        error_at(tok->str, "declear_node_ident() failure: 既に宣言済みです");
    }

    var = is_global ? new_gvar(tok, type) : new_lvar(tok, type);
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
            funcs[i++] = func_define(type);
            is_global = true;
        } else {
            Node *node = declaration_global(type);
        }
    }
    funcs[i] = NULL;
}

/*
 *  <declaration_global> = <declaration> ";"
 */
static Node *declaration_global(Type *type) {
    if (type->kind == TYPE_STRUCT && is_struct_create) {
        // 構造体の作成
        is_struct_create = false;
        expect(';');
        return new_node(ND_NULL);
    }
    Node *node = declaration(type);
    expect(';');
    return node;
}

/*
 *  <initialize> = <assign>
 */
static Node *initialize(Initializer *init, Node *node) {
    initialize2(init);
    return new_node_init(init, node);
}

static void initialize2(Initializer *init) {
    if (init->children) {
        expect('{');
        for (int i = 0; i < init->len; i++) {
            if (consume_nostep('}')) {
                (init->children + i)->expr = new_node_num(0);
                continue;
            }

            if (i > 0) expect(',');
            initialize2(init->children + i);
        }
        expect('}');
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
        node->var->type = type_suffix(node->var->type);
        // 新しい型のオフセットにする
        node->var->offset += sizeOfType(node->var->type) - sizeOfType(type);
    }
    // 変数
    if (consume('=')) {
        Initializer *init = new_initializer(node->var->type);
        return initialize(init, node);
    }

    return node;
}

/*
 *  <declaration> = <type_specifier> <declaration_var> ("," <declaration_var>)*
 */
static Node *declaration(Type *type) {
    Node *node = declaration_var(type);
    if (consume_nostep(';')) {
        return node;
    }

    Node *n = new_node(ND_SUGER);
    n->stmts = new_vec();
    vec_push(n->stmts, node);
    while (consume(',')) {
        vec_push(n->stmts, declaration_var(type));
    }
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
    t = type_suffix(t);
    new_struct_member(tok, t, type);
    expect(';');
    return type;
}

/*
 *  <type_specifier> = "int"
 *                   | "char"
 *                   | "void"
 *                   | "struct" <ident>
 *                   | "struct" <ident> "{" <struct_declaration>* "}"
 */
static Type *type_specifier() {
    expect_nostep(TK_TYPE);
    Type *type = token->type;
    next_token();
    if (type->kind == TYPE_STRUCT && consume('{')) {
        is_struct_create = true;
        if (is_global) {
            if (find_gstruct_type(type->name) != NULL) {
                error("find_gstruct_type() failure: %s構造体は既に宣言済みです。", type->name);
            }
        } else {
            if (find_lstruct_type(type->name) != NULL) {
                error("find_lstruct_type() failure: %s構造体は既に宣言済みです。", type->name);
            }
        }

        vec_push(is_global ? struct_global_lists : struct_local_lists, type);
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

    if (type->kind == TYPE_STRUCT) {
        Type *stype = find_lstruct_type(type->name);
        if (stype == NULL) {
            stype = find_gstruct_type(type->name);
            if (stype == NULL) {
                error("type_specifier() failure: %s構造体は宣言されていません。", type->name);
            }
        }
        type = stype;
    }

    return type;
}

/*
 *  <type_suffix> = "[" <num> "]" <type_suffix> | ε
 */
static Type *type_suffix(Type *type) {
    if (consume('[')) {
        int array_size = expect_number();
        expect(']');
        type = new_array_type(type_suffix(type), array_size);
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
    expect(TK_IDENT);
    Var *lvar = memory_alloc(sizeof(Var));
    lvar->name = tok->str;
    lvar->len = tok->len;
    lvar->type = type;
    lvar->offset = cur->offset + sizeOfType(lvar->type);
    if (consume_nostep('[')) {
        // ポインタとして受け取る
        lvar->type = type_suffix(lvar->type);
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

    expect(TK_IDENT);
    fn->name = my_strndup(tok->str, tok->len);
    fn->ret_type = type;
    expect('(');
    while (!consume(')')) {
        if (token->kind == TK_TYPE && token->type->kind == TYPE_VOID) {
            expect(TK_TYPE);
            continue;
        }

        if (cur != &head) {
            expect(',');
        }
        Var *p = declaration_param(cur);
        // 配列型は暗黙にポインターとして扱う
        if (p->type->kind == TYPE_ARRAY) {
            Type *t = p->type;
            p->type = new_ptr_type(t->ptr_to);
            p->offset += sizeOfType(p->type) - sizeOfType(t);
        }
        cur = cur->next = p;
    }

    fn->params = head.next;  // 前から見ていく
    locals = memory_alloc(sizeof(Var));
    struct_local_lists = new_vec();  // 関数毎に構造体を初期化
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
 *  <expr> = <assign> | <declaration>
 */
static Node *expr() {
    if (consume_nostep(TK_TYPE)) {
        Type *type = type_specifier();
        if (type->kind == TYPE_STRUCT && is_struct_create) {
            // 構造体の作成
            is_struct_create = false;
            return new_node(ND_NULL);
        }

        Node *node = declaration(type);
        return node;
    }

    return assign();
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
    Node *node = equality();

    for (;;) {
        if (consume(TK_LOGICAL_AND)) {
            node = new_binop(ND_LOGICAL_AND, node, equality());
        } else if (consume(TK_LOGICAL_OR)) {
            node = new_binop(ND_LOGICAL_OR, node, equality());
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
 *  <relational> = <add> ("<" <add> | "<=" <add> | ">" <add> | ">=" <add>)*
 */
static Node *relational() {
    Node *node = add();

    for (;;) {
        if (consume('<')) {
            node = new_binop(ND_LT, node, add());
        } else if (consume(TK_LE)) {
            node = new_binop(ND_LE, node, add());
        } else if (consume('>')) {
            node = new_binop(ND_LT, add(), node);
        } else if (consume(TK_GE)) {
            node = new_binop(ND_LE, add(), node);
        } else {
            return node;
        }
    }
}

// TODO: &, |, ^
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
 *  <mul> = <unary> ("*" <unary> | "/" <unary> | "%" <unary> )*
 */
static Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume('*')) {
            node = new_mul(node, unary());
        } else if (consume('/')) {
            node = new_div(node, unary());
        } else if (consume('%')) {
            node = new_mod(node, unary());
        } else {
            return node;
        }
    }
}

/*
 *  <unary> = "+"? <postfix>
 *          | "-"? <postfix>
 *          | "*" <unary>
 *          | "&" <postfix>
 *          | "sizeof" <unary>
 *          | "sizeof" "(" <type_specifier> <pointer> ")"
 *          | ("++" | "--") <postfix>
 *          | <postfix> ("++" | "--")
 *          | "!" <unary>
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
    } else if (consume(TK_SIZEOF)) {
        Token *tok = get_nafter_token(1);
        if (tok->kind == TK_TYPE) {
            expect('(');
            Type *t = type_specifier();
            t = pointer(t);
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
 *  <funcall> = "(" (<expr> ("," <expr>)*)? ")"
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
        vec_push(node->args, expr());
    }
    return node;
}

/*
 *  <primary> = "(" <expr> ")" | <num> | <string> | <ident> <funcall>?
 */
static Node *primary() {
    if (consume('(')) {
        Node *node = expr();
        expect(')');
        return node;
    }

    if (consume_nostep(TK_IDENT)) {
        Token *tok = token;
        next_token();
        Node *node;
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