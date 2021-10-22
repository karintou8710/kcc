#include "9cc.h"

static LVar *locals;

static LVar *new_lvar(Token *tok);
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
bool consume(int op) {
    if (token->kind != op) {
        return false;
    }
    next_token();
    return true;
}

bool equal(int op) {
    if (token->kind != op) {
        return false;
    }
    return true;
}

void expect(int op) {
    if (token->kind != op) {
        error_at(token->str, "'%s'ではありません", op);
    }
    next_token();
}

int expect_number() {
    if (token->kind != TK_NUM) {
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    next_token();
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

Node *new_node(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

Node *new_binop(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node *new_node_ident(Token *tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    LVar *lvar = find_lvar(tok);
    if (lvar) {
        node->lvar = lvar;
    } else {
        lvar = new_lvar(tok);
        node->lvar = lvar;
    }
    return node;
}

// ローカル変数を検索
LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next) {
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
            return var;
        }
    }
    return NULL;
}

// ローカル変数の作成
static LVar *new_lvar(Token *tok) {
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = tok->str;
    lvar->len = tok->len;
    lvar->offset = locals->offset + 8;
    locals = lvar;
    return lvar;
}

// 引数からローカル変数を作成する
static void create_lvar_from_params(LVar *params) {
    if (params) {
        LVar *lvar = calloc(1, sizeof(LVar));
        lvar->name = params->name;
        lvar->len = params->len;
        lvar->offset = locals->offset + 8;
        lvar->next = locals;
        locals = lvar;
        create_lvar_from_params(params->next);
    }
}

/* AST */

// program = func_define*
void program() {
    int i=0;
    while(!at_eof()) {
        funcs[i++] = func_define();
    }
    funcs[i] = NULL;
}

// func_define = ident "(" (ident ("," ident)*)? ")" compound_stmt
static Function *func_define() {
    Function *fn = calloc(1, sizeof(Function));
    Token *tok = token;
    LVar head = {};
    LVar *cur = &head;

    expect(TK_IDENT);
    fn->name = my_strndup(tok->str, tok->len);
    expect('(');
    while (!consume(')')) {
        if (cur != &head) {
            expect(',');
        }
        tok = token;
        expect(TK_IDENT);
        LVar *lvar = calloc(1, sizeof(LVar));
        lvar->name = tok->str;
        lvar->len = tok->len;
        lvar->offset = cur->offset + 8;
        cur = cur->next = lvar;
    }
    fn->params = head.next;
    locals = calloc(1, sizeof(LVar));
    create_lvar_from_params(fn->params);
    fn->body = compound_stmt();
    fn->locals = locals;
    return fn;
}

// compound_stmt = { stmt* }
static Node *compound_stmt() {
    expect('{');
    Node *node = new_node(ND_BLOCK);
    node->stmts = new_vec();
    while(!consume('}')) {
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
static Node *stmt() {
    Node *node;

    if (consume(TK_RETURN)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
        expect(';');
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
    } else if (equal('{')) {
        node = compound_stmt();
    } else {
        node = expr();
        expect(';');
    }
    
    return node;
}

// expr = assign
Node *expr() {
    return assign();
}

// assign = equality ("=" assign)?
static Node *assign() {
    Node *node = equality();
    if (consume('=')) {
        node = new_binop(ND_ASSIGN, node, assign());
    } else if (consume(TK_ADD_EQ)) {
        node = new_binop(ND_ASSIGN, node, new_binop(ND_ADD, node, equality()));
    } else if (consume(TK_SUB_EQ)) {
        node = new_binop(ND_ASSIGN, node, new_binop(ND_SUB, node, equality()));
    } else if (consume(TK_MUL_EQ)) {
        node = new_binop(ND_ASSIGN, node, new_binop(ND_MUL, node, equality()));
    } else if (consume(TK_DIV_EQ)) {
        node = new_binop(ND_ASSIGN, node, new_binop(ND_DIV, node, equality()));
    }
    return node;
}

// equality = relational ("==" relational | "!=" relational)*
static Node *equality() {
    Node *node = relational();

    for(;;) {
        if (consume(TK_EQ)) {
            node = new_binop(ND_EQ, node, relational());
        } else if (consume(TK_NE)) {
            node = new_binop(ND_NE, node, relational());
        } else {
            return node;
        }
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational() {
    Node *node = add();

    for(;;) {
        if (consume('<')) {
            node = new_binop(ND_LT, node, add());
        } else if (consume(TK_LE)) {
            node = new_binop(ND_LE, node, add());
        } else if(consume('>')) {
            node = new_binop(ND_LT, add(), node);
        } else if (consume(TK_GE)) {
            node = new_binop(ND_LE, add(), node);
        } else {
            return node;
        }
    }
}

// add = mul ("+" mul | "-" mul)*
static Node *add() {
    Node *node = mul();

    for(;;) {
        if (consume('+')) {
            node = new_binop(ND_ADD, node, mul());
        } else if (consume('-')) {
            node = new_binop(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

// mul = unary ("*" unary | "/" unary)*
static Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume('*')) {
            node = new_binop(ND_MUL, node, unary());
        } else if (consume('/')) {
            node = new_binop(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

// unary = ("+" | "-")? primary
static Node *unary() {
    if (consume('+')) {
        return primary();
    } else if (consume('-')) {
        return new_binop(ND_SUB, new_node_num(0), primary());
    } else {
        return primary();
    }
}

// funcall = ident "(" (expr ("," expr)*)? ")"
static Node *funcall(Token *tok) {
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

// primary = "(" expr ")" | num | IDENT | funcall
/* ※ funcallがBNFと一致していない */
static Node *primary() {
    if (consume('(')) {
        Node *node = expr();
        expect(')');
        return node;
    }

    if (token->kind == TK_IDENT) {
        Token *tok = token;
        next_token();
        Node *node;
        if (consume('(')) {
            // 関数の呼びだし
            node = funcall(tok);
        } else {
            node = new_node_ident(tok);
        }
        return node;
    }

    if (token->kind == TK_NUM) {
        return new_node_num(expect_number());
    }

    error("%sは不正なトークンです。", token->str);
}