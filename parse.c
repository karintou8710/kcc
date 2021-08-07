#include "9cc.h"

// 演算子の比較
bool consume(int op) {
    if (token->kind != op) {
        return false;
    }
    next_token();
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
        node->offset = lvar->offset;
    } else {
        lvar = calloc(1, sizeof(LVar));
        lvar->next = locals;
        lvar->name = tok->str;
        lvar->len = tok->len;
        lvar->offset = locals->offset + 8;
        node->offset = lvar->offset;
        locals = lvar;
    }
    return node;
}

LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next) {
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
            return var;
        }
    }
    return NULL;
}

// AST

void program() {
    int i=0;
    while(!at_eof()) {
        code[i++] = stmt();
    }
    code[i] = NULL;
}

Node *stmt() {
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
        if (token->kind != ';') {
            node->init = expr();
        }
        expect(';');
        if (token->kind != ';') {
            node->cond = expr();
        }
        expect(';');
        if (token->kind != ')') {
            node->inc = expr();
        }
        expect(')');
        node->body = stmt();
    } else {
        node = expr();
        expect(';');
    }
    
    return node;
}

Node *expr() {
    return assign();
}

Node *assign() {
    Node *node = equality();
    if (consume('=')) {
        node = new_binop(ND_ASSIGN, node, assign());
    }
    return node;
}

Node *equality() {
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

Node *relational() {
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

Node *add() {
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

Node *mul() {
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

Node *unary() {
    if (consume('+')) {
        return primary();
    } else if (consume('-')) {
        return new_binop(ND_SUB, new_node_num(0), primary());
    } else {
        return primary();
    }
}

Node *primary() {
    if (consume('(')) {
        Node *node = expr();
        expect(')');
        return node;
    }

    if (token->kind == TK_IDENT) {
        Node *node = new_node_ident(token);
        next_token();
        return node;
    }

    if (token->kind == TK_NUM) {
        return new_node_num(expect_number());
    }

    error("%sは不正なトークンです。", token->str);
}