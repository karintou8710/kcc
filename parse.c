#include "9cc.h"

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

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

Token *new_token(int kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startsWith(char *p, char *q) {
    return memcmp(p, q, strlen(q)) == 0;
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (startsWith(p, "==")) {
            cur = new_token(TK_EQ, cur, p, 2);
            p+=2;
            continue;
        }

        if (startsWith(p, "!=")) {
            cur = new_token(TK_NE, cur, p, 2);
            p+=2;
            continue;
        }

        if (startsWith(p, "<=")) {
            cur = new_token(TK_LE, cur, p, 2);
            p+=2;
            continue;
        }

        if (startsWith(p, ">=")) {
            cur = new_token(TK_GE, cur, p, 2);
            p+=2;
            continue;
        }

        if (strchr("+-*/=;()<>", *p)) {
            cur = new_token(*p, cur, p, 1);
            p++;
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }

        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            cur = new_token(TK_IF, cur, p, 2);
            p += 2;
            continue;
        }

        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
            cur = new_token(TK_ELSE, cur, p, 4);
            p += 4;
            continue;
        }

        // 小文字だけのローカル変数
        if ('a'<=*p && *p<='z') {
            cur = new_token(TK_IDENT, cur, p, 0);
            char *q = p;
            str_advanve(&p);
            cur->len = p - q;
            continue;
        }

        error("トークナイズできません");
    }

    
    new_token(TK_EOF, cur, p, 0);
    return head.next;
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

    printf("%c\n", token->kind);
    error("不正なトークンです。");
}