#include "9cc.h"

static LVar *locals;

static LVar *new_lvar(Token *tok, Type *type);
static void create_lvar_from_params(LVar *params);
LVar *find_lvar(Token *tok);

Node *new_add(Node *lhs, Node *rhs);
Node *new_sub(Node *lhs, Node *rhs);
Node *new_mul(Node *lhs, Node *rhs);
Node *new_div(Node *lhs, Node *rhs);

static Type *declaration_specifier();
static LVar *new_lvar(Token *tok, Type *type);
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
static Node *primary();

// 演算子の比較
bool consume(int op)
{
    if (token->kind != op)
    {
        return false;
    }
    next_token();
    return true;
}

bool consume_nostep(int op)
{
    if (token->kind != op)
    {
        return false;
    }
    return true;
}

void expect(int op)
{
    if (token->kind != op)
    {
        if (op == TK_TYPE)
        {
            error("適当な位置に型がありません");
        }

        error_at(token->str, "'%s'ではありません", op);
    }
    next_token();
}

void expect_nostep(int op)
{
    if (token->kind != op)
    {
        if (op == TK_TYPE)
        {
            error("適当な位置に型がありません");
        }

        error_at(token->str, "'%s'ではありません", op);
    }
}

//
int expect_number()
{
    if (token->kind != TK_NUM)
    {
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    next_token();
    return val;
}

// 文末
bool at_eof()
{
    return token->kind == TK_EOF;
}

// sizeofの実装
int sizeOfNode(Node *node)
{
    add_type(node);
    return sizeOfType(node->type);
}

// ローカル変数の作成
static LVar *new_lvar(Token *tok, Type *type)
{
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = tok->str;
    lvar->len = tok->len;
    lvar->type = type;
    lvar->offset = locals->offset + sizeOfType(type);
    locals = lvar;
    return lvar;
}

// 引数からローカル変数を作成する
static void create_lvar_from_params(LVar *params)
{
    if (params)
    {
        LVar *lvar = calloc(1, sizeof(LVar));
        lvar->name = params->name;
        lvar->len = params->len;
        lvar->type = new_type(TYPE_INT);
        lvar->offset = locals->offset + sizeOfType(lvar->type);
        lvar->next = locals;

        locals = lvar;
        create_lvar_from_params(params->next);
    }
}

// ローカル変数を検索
LVar *find_lvar(Token *tok)
{
    for (LVar *var = locals; var; var = var->next)
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
Node *new_node(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

// 演算子ノード作成
Node *new_binop(NodeKind kind, Node *lhs, Node *rhs)
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
Node *new_add(Node *lhs, Node *rhs)
{
    add_type(lhs);
    add_type(rhs);

    // enum TypeKind の順番にする (lhs <= rhs)
    if (lhs->type->kind > rhs->type->kind)
    {
        swap((void **)&lhs, (void **)&rhs);
    }

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_ADD;
    node->lhs = lhs;
    node->rhs = rhs;

    if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_INT)
    {
        return node;
    }

    if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_PTR)
    {
        node->lhs = new_mul(lhs, new_node_num(rhs->type->ptr_to->size));
        add_type(node->lhs);
        return node;
    }

    if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_ARRAY)
    {
        node->lhs = new_mul(lhs, new_node_num(rhs->type->ptr_to->size));
        add_type(node->lhs);
        return node;
    }

    error("実行できないコードです");
}

Node *new_sub(Node *lhs, Node *rhs)
{
    add_type(lhs);
    add_type(rhs);

    // lhsとrhsの順番に関係あり

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_SUB;
    node->lhs = lhs;
    node->rhs = rhs;

    if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_INT)
    {
        return node;
    }

    if (lhs->type->kind == TYPE_PTR && rhs->type->kind == TYPE_INT)
    {
        node->rhs = new_mul(rhs, new_node_num(lhs->type->ptr_to->size));
        add_type(node->rhs);
        return node;
    }

    if (lhs->type->kind == TYPE_ARRAY && rhs->type->kind == TYPE_INT)
    {
        node->rhs = new_mul(rhs, new_node_num(lhs->type->ptr_to->size));
        add_type(node->rhs);
        return node;
    }

    error("実行できないコードです");
}

Node *new_mul(Node *lhs, Node *rhs)
{
    add_type(lhs);
    add_type(rhs);

    // enum TypeKind の順番にする (lhs <= rhs)
    if (lhs->type->kind > rhs->type->kind)
    {
        swap((void **)&lhs, (void **)&rhs);
    }

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_MUL;
    node->lhs = lhs;
    node->rhs = rhs;

    if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_INT)
    {
        return node;
    }

    error("実行できないコードです");
}

Node *new_div(Node *lhs, Node *rhs)
{
    add_type(lhs);
    add_type(rhs);

    // lhsとrhsの順番に関係あり (lhs <= rhs)

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_DIV;
    node->lhs = lhs;
    node->rhs = rhs;

    if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_INT)
    {
        return node;
    }

    error("実行できないコードです");
}

// 数値ノードを作成
Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// ローカル変数を宣言
Node *declear_node_ident(Token *tok, Type *type)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    LVar *lvar = find_lvar(tok);
    if (lvar)
    {
        error("既に宣言済みです");
    }

    lvar = new_lvar(tok, type);
    node->lvar = lvar;
    return node;
}

// ローカル変数のノードを取得
Node *get_node_ident(Token *tok)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    LVar *lvar = find_lvar(tok);
    if (!lvar)
    {
        error("宣言されていません\n");
    }

    node->lvar = lvar;
    return node;
}

/*************************************/
/******                         ******/
/******           AST           ******/
/******                         ******/
/*************************************/

// program = func_define*
void program()
{
    int i = 0;
    while (!at_eof())
    {
        funcs[i++] = func_define();
    }
    funcs[i] = NULL;
}

// func_define = declaration_specifier ident "(" (declaration_specifier ident ("," declaration_specifier ident)*)? ")" compound_stmt
static Function *func_define()
{
    Type *type = declaration_specifier();

    Function *fn = calloc(1, sizeof(Function));
    Token *tok = token;
    LVar head = {};
    LVar *cur = &head;

    expect(TK_IDENT);
    fn->name = my_strndup(tok->str, tok->len);
    expect('(');
    while (!consume(')'))
    {
        if (cur != &head)
        {
            expect(',');
        }
        Type *type = declaration_specifier();
        tok = token;
        expect(TK_IDENT);
        LVar *lvar = calloc(1, sizeof(LVar));
        lvar->name = tok->str;
        lvar->len = tok->len;
        lvar->type = type;
        lvar->offset = cur->offset + sizeOfType(lvar->type);
        cur = cur->next = lvar;
    }
    fn->params = head.next;
    locals = calloc(1, sizeof(LVar));
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

/* stmt = expr ";"
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
    else
    {
        node = expr();
        expect(';');
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

// expr = assign | declaration_specifier ident ("[" num "]")?
// TODO: とりあえず一次元の配列だけを定義する
static Node *expr()
{

    if (consume_nostep(TK_TYPE))
    {
        Type *type = declaration_specifier();
        Node *node = declear_node_ident(token, type);
        next_token();
        if (consume('['))
        {
            int array_size = expect_number();
            node->lvar->type = new_array_type(node->lvar->type, array_size);
            node->lvar->offset = sizeOfType(node->lvar->type);
            expect(']');
        }
        return node;
    }

    return assign();
}

// assign = equality ("=" assign)?
static Node *assign()
{
    Node *node = equality();
    if (consume('='))
    {
        node = new_binop(ND_ASSIGN, node, assign());
    }
    else if (consume(TK_ADD_EQ))
    {
        node = new_binop(ND_ASSIGN, node, new_binop(ND_ADD, node, equality()));
    }
    else if (consume(TK_SUB_EQ))
    {
        node = new_binop(ND_ASSIGN, node, new_binop(ND_SUB, node, equality()));
    }
    else if (consume(TK_MUL_EQ))
    {
        node = new_binop(ND_ASSIGN, node, new_binop(ND_MUL, node, equality()));
    }
    else if (consume(TK_DIV_EQ))
    {
        node = new_binop(ND_ASSIGN, node, new_binop(ND_DIV, node, equality()));
    }
    return node;
}

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
        else
        {
            return node;
        }
    }
}

/* unary = "+"? primary
 *       | "-"? primary
 *       | "*" primary   ("*" unaryでもいい？)
 *       | "&" primary
 *       | "sizeof" unary
 */
static Node *unary()
{
    if (consume('+'))
    {
        return primary();
    }
    else if (consume('-'))
    {
        return new_sub(new_node_num(0), primary());
    }
    else if (consume('*'))
    {
        Node *node = new_node(ND_DEREF);
        Node *top_node = node;
        while (consume('*'))
        {
            Node *n = new_node(ND_DEREF);
            node->lhs = n;
            n->rhs = node;
            node = n;
        }
        node->lhs = primary();

        add_type(node->lhs);

        while (1)
        {
            Type *ty = node->lhs->type;
            if (!ty->ptr_to)
            {
                error("derefに失敗しました");
            }
            node->type = ty->ptr_to;
            if (!node->rhs)
                break;
            node = node->rhs;
        }

        return top_node;
    }
    else if (consume('&'))
    {
        Node *node = new_node(ND_ADDR);
        node->lhs = primary();
        add_type(node->lhs);
        node->lhs->type = new_ptr_type(node->lhs->type);
        return node;
    }
    else if (consume(TK_SIZEOF))
    {
        Node *node = new_node_num(sizeOfNode(unary()));
        return node;
    }

    return primary();
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

// primary = "(" expr ")" | num | ident funcall?
/* ※ funcallがBNFと一致していない */
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
            // 関数の呼びだし
            node = funcall(tok);
        }
        else
        {
            node = get_node_ident(tok);
        }
        return node;
    }

    if (consume_nostep(TK_NUM))
    {
        return new_node_num(expect_number());
    }

    error("%sは不正なトークンです。", token->str);
}