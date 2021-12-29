#include "9cc.h"

/*
 * ローカル変数を単方向の連結リストで持つ
 * localsは後に出たローカル変数のポインタを持つ
 */
static Var *locals;

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
static Var *find_var(Token *tok, bool is_global);

/* AST */
static Type *declaration_specifier();
static Node *declaration_global(Type *type);
static Node *declaration_var(Type *type, bool is_global);
static Var *declaration_param(Var *cur);
static Function *func_define();
static Node *compound_stmt();
static Node *stmt();
static Node *expr();
static Node *assign();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *array_suffix();
static Type *type_suffix(Type *type);
static Node *primary();

/* 指定された演算子が来る可能性がある */
static bool consume(int op)
{
    if (token->kind != op)
    {
        return false;
    }
    next_token();
    return true;
}

static bool consume_nostep(int op)
{
    if (token->kind != op)
    {
        return false;
    }
    return true;
}

/* 指定された演算子が必ず来る */
static void expect(int op)
{
    if (token->kind != op)
    {
        if (op == TK_TYPE)
        {
            error("%d 適当な位置に型がありません", op);
        }
        error_at(token->str, "適切な演算子ではありません");
    }
    next_token();
}

static void expect_nostep(int op)
{
    if (token->kind != op)
    {
        if (op == TK_TYPE)
        {
            error("%d 適当な位置に型がありません", op);
        }

        error_at(token->str, "適切な演算子ではありません");
    }
}

static int expect_number()
{
    if (token->kind != TK_NUM)
    {
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    next_token();
    return val;
}

static bool at_eof()
{
    return token->kind == TK_EOF;
}

/* ノードを引数にもつsizeofの実装 */
static int sizeOfNode(Node *node)
{
    add_type(node);
    return sizeOfType(node->type);
}

/* ローカル変数の作成 */
static Var *new_lvar(Token *tok, Type *type)
{
    Var *lvar = calloc(1, sizeof(Var));
    lvar->next = locals;
    lvar->name = my_strndup(tok->str, tok->len);
    lvar->len = tok->len;
    lvar->type = type;
    lvar->offset = locals->offset + sizeOfType(type);
    locals = lvar; // localsを新しいローカル変数に更新
    return lvar;
}

static Var *new_gvar(Token *tok, Type *type)
{
    Var *gvar = calloc(1, sizeof(Var));
    gvar->next = globals;
    gvar->name = my_strndup(tok->str, tok->len);
    gvar->len = tok->len;
    gvar->type = type;
    gvar->is_global = true;
    globals = gvar; // globalsを新しいグローバル変数に更新
    return gvar;
}

// TODO: 引数に適切な型をつけるようにする
/* 引数からローカル変数を作成する(前から見ていく) */
static void create_lvar_from_params(Var *params)
{
    if (!params)
        return;

    Var *lvar = calloc(1, sizeof(Var));
    lvar->name = params->name;
    lvar->len = params->len;
    lvar->type = params->type;
    lvar->offset = locals->offset + sizeOfType(lvar->type);
    lvar->next = locals;

    locals = lvar;
    create_lvar_from_params(params->next);
}

/* 既に定義されたローカル変数を検索 */
static Var *find_var(Token *tok, bool is_global)
{
    Var *vars = is_global ? globals : locals;
    for (Var *var = vars; var; var = var->next)
    {
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
        {
            return var;
        }
    }
    return NULL;
}

/*************************************/
/******                         ******/
/******        NEW_NODE         ******/
/******                         ******/
/*************************************/

// ノード作成
static Node *new_node(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

// 演算子ノード作成
static Node *new_binop(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

/*
 * 演算子には型のキャストがある
 */
static Node *new_add(Node *lhs, Node *rhs)
{
    // 型を伝搬する
    add_type(lhs);
    add_type(rhs);

    // enum TypeKind の順番にする (lhs <= rhs)
    if (lhs->type->kind > rhs->type->kind)
    {
        swap((void **)&lhs, (void **)&rhs);
    }

    Node *node = new_binop(ND_ADD, lhs, rhs);

    if (is_numtype(lhs->type->kind) && is_numtype(rhs->type->kind))
    {
        return node;
    }

    if (is_numtype(lhs->type->kind) && rhs->type->kind == TYPE_PTR)
    {
        node->lhs = new_mul(lhs, new_node_num(rhs->type->ptr_to->size));
        add_type(node->lhs);
        return node;
    }

    if (is_numtype(lhs->type->kind) && rhs->type->kind == TYPE_ARRAY)
    {
        // ポインター型として演算
        node->lhs = new_mul(lhs, new_node_num(rhs->type->ptr_to->size));
        add_type(node->lhs);
        return node;
    }

    error("実行できない型による演算です(ADD)");
}

static Node *new_sub(Node *lhs, Node *rhs)
{
    add_type(lhs);
    add_type(rhs);

    // lhsとrhsの順番に関係あり
    Node *node = new_binop(ND_SUB, lhs, rhs);

    if (is_numtype(lhs->type->kind) && is_numtype(rhs->type->kind))
    {
        return node;
    }

    if (lhs->type->kind == TYPE_PTR && is_numtype(rhs->type->kind))
    {
        node->rhs = new_mul(rhs, new_node_num(lhs->type->ptr_to->size));
        add_type(node->rhs);
        return node;
    }

    if (lhs->type->kind == TYPE_ARRAY && is_numtype(rhs->type->kind))
    {
        // ポインター型として演算
        node->rhs = new_mul(rhs, new_node_num(lhs->type->ptr_to->size));
        add_type(node->rhs);
        return node;
    }

    error("実行できない型による演算です(SUB)");
}

static Node *new_mul(Node *lhs, Node *rhs)
{
    add_type(lhs);
    add_type(rhs);

    // enum TypeKind の順番にする (lhs <= rhs)
    if (lhs->type->kind > rhs->type->kind)
    {
        swap((void **)&lhs, (void **)&rhs);
    }

    Node *node = new_binop(ND_MUL, lhs, rhs);

    if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_INT)
    {
        return node;
    }

    error("実行できない型による演算です(MUL)");
}

static Node *new_div(Node *lhs, Node *rhs)
{
    add_type(lhs);
    add_type(rhs);

    // lhsとrhsの順番に関係あり (lhs <= rhs)
    Node *node = new_binop(ND_DIV, lhs, rhs);

    if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_INT)
    {
        return node;
    }

    error("実行できない型による演算です(DIV)");
}

static Node *new_mod(Node *lhs, Node *rhs)
{
    add_type(lhs);
    add_type(rhs);

    // lhsとrhsの順番に関係あり (lhs <= rhs)
    Node *node = new_binop(ND_MOD, lhs, rhs);

    if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_INT)
    {
        return node;
    }

    error("実行できない型による演算です(MOD)");
}

static Node *new_assign(Node *lhs, Node *rhs)
{
    add_type(lhs);
    add_type(rhs);

    Node *node = new_binop(ND_ASSIGN, lhs, rhs);

    return node;
}

/* 数値ノードを作成 */
static Node *new_node_num(int val)
{
    Node *node = new_node(ND_NUM);
    node->val = val;
    return node;
}

/* 変数を宣言 */
static Node *declear_node_ident(Token *tok, Type *type, bool is_global)
{
    Node *node = new_node(ND_VAR);
    Var *var = find_var(tok, is_global);
    if (var)
    {
        error("既に宣言済みです");
    }

    var = is_global ? new_gvar(tok, type) : new_lvar(tok, type);
    node->var = var;
    return node;
}

// 変数のノードを取得
static Node *get_node_ident(Token *tok)
{
    Node *node = new_node(ND_VAR);
    Var *var;
    var = find_var(tok, false); // ローカル変数を取得
    if (!var)
    {
        var = find_var(tok, true); // グローバル変数から取得
        if (!var) {
            error("宣言されていません");
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

// <program> = ( <declaration_global> | <func_define> )*
// 関数かどうかの先読みが必要
void program()
{
    int i = 0;
    while (!at_eof())
    {
        Type *type = declaration_specifier();
        Token *t = get_nafter_token(1);
        if (t->kind == '(') {
            funcs[i++] = func_define(type);
        } else {
            Node *node = declaration_global(type);
        }
        
    }
    funcs[i] = NULL;
}

// <declaration_global> = <declaration_var> ";"
static Node *declaration_global(Type *type) {
    Node *node = declaration_var(type, true);
    expect(';');
    return node;
}

// <initialize>  = "{" <initialize> (","  <initialize>)* "}"
//               | <assign>
static Node *initialize() {
    Node *node = NULL;
    // TODO 配列の初期化式
    // if (consume('{')) {
    //     node = initialize();
    //     while (consume(',')) {
    //         node = initialize();
    //     }
    //     expect('}');
    //     return node;
    // }

    return assign();   
}

// declaration_var = declaration_specifier ident type_suffix ("=" initialize)?
static Node *declaration_var(Type *type, bool is_global) {
    Node *node = declear_node_ident(token, type, is_global);
    next_token();
    if (consume_nostep('['))
    {
        // 配列
        node->var->type = type_suffix(node->var->type);
        // 新しい型のオフセットにする
        node->var->offset += sizeOfType(node->var->type) - sizeOfType(type);

        return node;
    }
    // 変数
    if (consume('='))
    {
        return new_assign(node , initialize()); 
    }

    return node;
}

// declaration_specifier = int "*"*
static Type *declaration_specifier()
{
    expect_nostep(TK_TYPE);
    Type *type = token->type;
    next_token();
    while (consume('*'))
    {
        Type *t = new_ptr_type(type);
        type = t;
    }
    return type;
}

// type_suffix = "[" num "]" type_suffix | ε
static Type *type_suffix(Type *type)
{
    if (consume('['))
    {
        int array_size = expect_number();
        expect(']');
        type = new_array_type(type_suffix(type), array_size);
    }

    return type;
}

// declaration_param = declaration_specifier ident type_suffix
static Var *declaration_param(Var *cur) {
    Type *type = declaration_specifier();
    Token *tok = token;
    expect(TK_IDENT);
    Var *lvar = calloc(1, sizeof(Var));
    lvar->name = tok->str;
    lvar->len = tok->len;
    lvar->type = type;
    lvar->offset = cur->offset + sizeOfType(lvar->type);
    if (consume_nostep('['))
    {
        // ポインタとして受け取る
        lvar->type = type_suffix(lvar->type);
        // 新しい型のオフセットにする
        lvar->offset += sizeOfType(lvar->type) - sizeOfType(type);
    }
    return lvar;
}

// func_define = declaration_specifier ident "("
// (declaration_param ("," declaration_param)* )? ")" compound_stmt
//
//
static Function *func_define(Type *type)
{
    Function *fn = calloc(1, sizeof(Function));
    Token *tok = token;
    Var head = {};
    Var *cur = &head; // 引数の単方向連結リスト

    expect(TK_IDENT);
    fn->name = my_strndup(tok->str, tok->len);
    expect('(');
    while (!consume(')'))
    {
        if (cur != &head)
        {
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

    fn->params = head.next; // 前から見ていく
    locals = calloc(1, sizeof(Var));
    locals->offset = 0;
    create_lvar_from_params(fn->params);
    fn->body = compound_stmt();
    fn->locals = locals;
    return fn;
}

// compound_stmt = { stmt* }
static Node *compound_stmt()
{
    expect('{');
    Node *node = new_node(ND_BLOCK);
    node->stmts = new_vec();
    while (!consume('}'))
    {
        vec_push(node->stmts, stmt());
    }
    return node;
}

// TODO: do~while,continue,break,switch,else if,
/* stmt = expr? ";"
 *     | "return" expr ";"
 *      | "if" "(" expr ")" stmt ("else" stmt)?
 *      | "while" "(" expr ")" stmt
 *      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
 *      | compound_stmt
 */
static Node *stmt()
{
    Node *node;

    if (consume(TK_RETURN))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
        expect(';');
    }
    else if (consume(TK_IF))
    {
        node = new_node(ND_IF);
        expect('(');
        node->cond = expr();
        expect(')');
        node->then = stmt();
        if (consume(TK_ELSE))
        {
            node->els = stmt();
        }
    }
    else if (consume(TK_WHILE))
    {
        node = new_node(ND_WHILE);
        expect('(');
        node->cond = expr();
        expect(')');
        node->body = stmt();
    }
    else if (consume(TK_FOR))
    {
        node = new_node(ND_FOR);
        expect('(');
        if (!consume(';'))
        {
            node->init = expr();
            expect(';');
        }
        if (!consume(';'))
        {
            node->cond = expr();
            expect(';');
        }
        if (!consume(')'))
        {
            node->inc = expr();
            expect(')');
        }
        node->body = stmt();
    }
    else if (consume_nostep('{'))
    {
        node = compound_stmt();
    }
    else if (consume(';'))
    {
        node = new_node(ND_BLOCK);
        node->stmts = new_vec();
    }
    else
    {
        node = expr();
        expect(';');
    }

    return node;
}

// TODO: とりあえず一次元の配列だけを定義する
// TODO: 多次元配列に対応
// exprは一つの式で型の伝搬は大体ここまでありそう
// expr = assign | declaration_var
static Node *expr()
{
    if (consume_nostep(TK_TYPE))
    {
        Type *type = declaration_specifier();
        Node *node = declaration_var(type, false);
        return node;
    }

    return assign();
}

// TODO: %=, ++, --, ?:, <<=, >>=, &=, ^=, |=, ","
// assign = equality ("=" assign)?
//        | equality ( "+=" | "-=" | "*=" | "/=" | "%=" ) equality
static Node *assign()
{
    Node *node = equality();
    if (consume('='))
    {
        node = new_assign(node, assign());
    }
    else if (consume(TK_ADD_EQ))
    {
        node = new_assign(node, new_add(node, equality()));
    }
    else if (consume(TK_SUB_EQ))
    {
        node = new_assign(node, new_sub(node, equality()));
    }
    else if (consume(TK_MUL_EQ))
    {
        node = new_assign(node, new_mul(node, equality()));
    }
    else if (consume(TK_DIV_EQ))
    {
        node = new_assign(node, new_div(node, equality()));
    }
    else if (consume(TK_MOD_EQ))
    {
        node = new_assign(node, new_mod(node, equality()));
    }
    return node;
}

// TODO: &&, ||
// equality = relational ("==" relational | "!=" relational)*
static Node *equality()
{
    Node *node = relational();

    for (;;)
    {
        if (consume(TK_EQ))
        {
            node = new_binop(ND_EQ, node, relational());
        }
        else if (consume(TK_NE))
        {
            node = new_binop(ND_NE, node, relational());
        }
        else
        {
            return node;
        }
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational()
{
    Node *node = add();

    for (;;)
    {
        if (consume('<'))
        {
            node = new_binop(ND_LT, node, add());
        }
        else if (consume(TK_LE))
        {
            node = new_binop(ND_LE, node, add());
        }
        else if (consume('>'))
        {
            node = new_binop(ND_LT, add(), node);
        }
        else if (consume(TK_GE))
        {
            node = new_binop(ND_LE, add(), node);
        }
        else
        {
            return node;
        }
    }
}

// TODO: &, |, ^
// add = mul ("+" mul | "-" mul)*
static Node *add()
{
    Node *node = mul();

    for (;;)
    {
        if (consume('+'))
        {
            node = new_add(node, mul());
        }
        else if (consume('-'))
        {
            node = new_sub(node, mul());
        }
        else
        {
            return node;
        }
    }
}

// mul = unary ("*" unary | "/" unary)*
static Node *mul()
{
    Node *node = unary();

    for (;;)
    {
        if (consume('*'))
        {
            node = new_mul(node, unary());
        }
        else if (consume('/'))
        {
            node = new_div(node, unary());
        }
        else if (consume('%'))
        {
            node = new_mod(node, unary());
        }
        else
        {
            return node;
        }
    }
}

// TODO: !(否定),
/* unary = "+"? array_suffix
 *       | "-"? array_suffix
 *       | "*" array_suffix   ("*" unaryでもいい？)
 *       | "&" array_suffix
 *       | "sizeof" unary
 *       | ("++" | "--") array_suffix
 */
static Node *unary()
{
    if (consume('+'))
    {
        return array_suffix();
    }
    else if (consume('-'))
    {
        return new_sub(new_node_num(0), array_suffix());
    }
    else if (consume('*'))
    {
        Node *node = new_node(ND_DEREF);
        node->lhs = unary();
        add_type(node->lhs);
        add_type(node);
        return node;
    }
    else if (consume('&'))
    {
        Node *node = new_node(ND_ADDR);
        node->lhs = array_suffix();
        add_type(node->lhs);
        node->lhs->type = new_ptr_type(node->lhs->type);
        return node;
    }
    else if (consume(TK_SIZEOF))
    {
        Node *node = new_node_num(sizeOfNode(unary()));
        return node;
    }
    else if (consume(TK_INC))
    {
        Node *node = array_suffix();
        return new_assign(node, new_add(node, new_node_num(1)));
    }
    else if (consume(TK_DEC))
    {
        Node *node = array_suffix();
        return new_assign(node, new_sub(node, new_node_num(1)));
    }

    Node *node = array_suffix();
    if (consume(TK_INC))
    {
        // 先に+1して保存してから-1する
        return new_sub(new_assign(node, new_add(node, new_node_num(1))), new_node_num(1));
    }
    else if (consume(TK_DEC))
    {
        // 先に+-1して保存してから+1する
        return new_add(new_assign(node, new_sub(node, new_node_num(1))), new_node_num(1));
    }

    return node;
}

// array_suffix = primary ("[" expr "]")*
static Node *array_suffix()
{
    Node *node = primary();

    while (consume('['))
    {
        Node *deref = new_node(ND_DEREF);
        deref->lhs = new_add(node, expr());
        add_type(deref->lhs);
        add_type(deref);
        expect(']');
        node = deref;
    }

    return node;
}

// funcall = "(" (expr ("," expr)*)? ")"
static Node *funcall(Token *tok)
{
    expect('(');
    Node *node = new_node(ND_CALL);
    node->fn_name = my_strndup(tok->str, tok->len);
    node->args = new_vec();
    while (!consume(')'))
    {
        if (node->args->len != 0)
        {
            expect(',');
        }
        vec_push(node->args, expr());
    }
    return node;
}

// primary = "(" expr ")" | num | string | ident funcall?
static Node *primary()
{
    if (consume('('))
    {
        Node *node = expr();
        expect(')');
        return node;
    }

    if (consume_nostep(TK_IDENT))
    {
        Token *tok = token;
        next_token();
        Node *node;
        if (consume_nostep('('))
        {
            node = funcall(tok);
        }
        else
        {
            node = get_node_ident(tok);
        }
        return node;
    }

    if (consume_nostep(TK_STRING))
    {
        Node *node = new_node(ND_STRING);
        node->str_literal = token->str;
        node->val = token->str_literal_index;
        next_token();
        return node;
    }

    if (consume_nostep(TK_NUM))
    {
        return new_node_num(expect_number());
    }

    error("%sは不正なトークンです。", token->str);
}