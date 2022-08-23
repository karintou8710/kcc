#include "kcc.h"

static void printf_with_space(int depth, char *fmt, ...) {
    va_list ap;
    int space_width = 4;
    va_start(ap, fmt);
    for (int i = 0; i < depth; i++) {
        fprintf(stderr, "|%*s", space_width - 1, "");
    }
    vfprintf(stderr, fmt, ap);
}

void print_node_kind(NodeKind kind) {
    fprintf(stderr, "nodekind -> ");
    if (kind == ND_ADD)
        fprintf(stderr, "ND_ADD");  // +
    else if (kind == ND_SUB)
        fprintf(stderr, "ND_SUB");  // -
    else if (kind == ND_MUL)
        fprintf(stderr, "ND_MUL");  // *
    else if (kind == ND_DIV)
        fprintf(stderr, "ND_DIV");  // /
    else if (kind == ND_MOD)
        fprintf(stderr, "ND_MOD");  // %
    else if (kind == ND_ASSIGN)
        fprintf(stderr, "ND_ASSIGN");  // =
    else if (kind == ND_EQ)
        fprintf(stderr, "ND_EQ");  // ==
    else if (kind == ND_NE)
        fprintf(stderr, "ND_NE");  // !=
    else if (kind == ND_LT)
        fprintf(stderr, "ND_LT");  // <
    else if (kind == ND_LE)
        fprintf(stderr, "ND_LE");  // <=
    else if (kind == ND_AND)
        fprintf(stderr, "ND_AND");  // &
    else if (kind == ND_OR)
        fprintf(stderr, "ND_OR");  // |
    else if (kind == ND_XOR)
        fprintf(stderr, "ND_XOR");  // ^
    else if (kind == ND_VAR)
        fprintf(stderr, "ND_VAR");  // local var
    else if (kind == ND_NUM)
        fprintf(stderr, "ND_NUM");  // num
    else if (kind == ND_RETURN)
        fprintf(stderr, "ND_RETURN");  // return
    else if (kind == ND_IF)
        fprintf(stderr, "ND_IF");  // if
    else if (kind == ND_ELSE)
        fprintf(stderr, "ND_ELSE");  // else
    else if (kind == ND_FOR)
        fprintf(stderr, "ND_FOR");  // for
    else if (kind == ND_WHILE)
        fprintf(stderr, "ND_WHILE");  // while
    else if (kind == ND_BLOCK)
        fprintf(stderr, "ND_BLOCK");  // block {}
    else if (kind == ND_CALL)
        fprintf(stderr, "ND_CALL");  // call
    else if (kind == ND_ADDR)
        fprintf(stderr, "ND_ADDR");  // & アドレス
    else if (kind == ND_DEREF)
        fprintf(stderr, "ND_DEREF");  // * ポインタ
    else if (kind == ND_STRING)
        fprintf(stderr, "ND_STRING");  // string literal
    else if (kind == ND_CONTINUE)
        fprintf(stderr, "ND_CONTINUE");  // continue
    else if (kind == ND_BREAK)
        fprintf(stderr, "ND_BREAK");  // break
    else if (kind == ND_LOGICAL_NOT)
        fprintf(stderr, "ND_LOGICAL_NOT");  // !
    else if (kind == ND_LOGICAL_AND)
        fprintf(stderr, "ND_LOGICAL_AND");  // &&
    else if (kind == ND_LOGICAL_OR)
        fprintf(stderr, "ND_LOGICAL_OR");  // ||
    else if (kind == ND_SUGER)
        fprintf(stderr, "ND_SUGER");  // suger
    else if (kind == ND_NULL)
        fprintf(stderr, "ND_NULL");  // null
    else if (kind == ND_STRUCT_MEMBER)
        fprintf(stderr, "ND_STRUCT_MEMBER");  // struct member
    else if (kind == ND_TERNARY)
        fprintf(stderr, "ND_TERNARY");  // 3項演算子
    else if (kind == ND_NOT)
        fprintf(stderr, "ND_NOT");  // ~
    else if (kind == ND_LSHIFT)
        fprintf(stderr, "ND_LSHIFT");  // ~
    else if (kind == ND_CAST)
        fprintf(stderr, "ND_CAST");  // cast
    else if (kind == ND_STMT_EXPR)
        fprintf(stderr, "ND_STMT_EXPR");  // stmt in expr
    else if (kind == ND_DO_WHILE)
        fprintf(stderr, "ND_DO_WHILE");  // do ... while
    else if (kind == ND_SWITCH)
        fprintf(stderr, "ND_SWITCH");  // switch
    else if (kind == ND_CASE)
        fprintf(stderr, "ND_CASE");  // case
    else if (kind == ND_DEFAULT)
        fprintf(stderr, "ND_DEFAULT");  // default
    else if (kind == ND_GOTO)
        fprintf(stderr, "ND_GOTO");  // goto
    else if (kind == ND_LABEL)
        fprintf(stderr, "ND_LABEL");  // label
    else
        error("print_node_kind() failure: 未定義ノード種");

    fprintf(stderr, "\n");
}

void print_token_kind(TokenKind kind) {
    fprintf(stderr, "tokenkind -> ");
    if (kind == TK_NUM)
        fprintf(stderr, "TK_NUM");
    else if (kind == TK_IDENT)
        fprintf(stderr, "TK_IDENT");
    else if (kind == TK_EQ)
        fprintf(stderr, "TK_EQ");
    else if (kind == TK_NE)
        fprintf(stderr, "TK_NE");
    else if (kind == TK_LE)
        fprintf(stderr, "TK_LE");
    else if (kind == TK_GE)
        fprintf(stderr, "TK_GE");
    else if (kind == TK_ADD_EQ)
        fprintf(stderr, "TK_ADD_EQ");
    else if (kind == TK_SUB_EQ)
        fprintf(stderr, "TK_SUB_EQ");
    else if (kind == TK_MUL_EQ)
        fprintf(stderr, "TK_MUL_EQ");
    else if (kind == TK_DIV_EQ)
        fprintf(stderr, "TK_DIV_EQ");
    else if (kind == TK_MOD_EQ)
        fprintf(stderr, "TK_MOD_EQ");
    else if (kind == TK_MOD_EQ)
        fprintf(stderr, "TK_AND_EQ");
    else if (kind == TK_AND_EQ)
        fprintf(stderr, "TK_OR_EQ");
    else if (kind == TK_OR_EQ)
        fprintf(stderr, "TK_XOR_EQ");
    else if (kind == TK_XOR_EQ)
        fprintf(stderr, "TK_LSHIFT_EQ");
    else if (kind == TK_LSHIFT_EQ)
        fprintf(stderr, "TK_LSHIFT_EQ");
    else if (kind == TK_RSHIFT_EQ)
        fprintf(stderr, "TK_RSHIFT_EQ");
    else if (kind == TK_INC)
        fprintf(stderr, "TK_INC");
    else if (kind == TK_DEC)
        fprintf(stderr, "TK_DEC");
    else if (kind == TK_RETURN)
        fprintf(stderr, "TK_RETURN");
    else if (kind == TK_IF)
        fprintf(stderr, "TK_IF");
    else if (kind == TK_ELSE)
        fprintf(stderr, "TK_ELSE");
    else if (kind == TK_FOR)
        fprintf(stderr, "TK_FOR");
    else if (kind == TK_WHILE)
        fprintf(stderr, "TK_WHILE");
    else if (kind == TK_EOF)
        fprintf(stderr, "TK_EOF");
    else if (kind == TK_TYPE)
        fprintf(stderr, "TK_TYPE");
    else if (kind == TK_SIZEOF)
        fprintf(stderr, "TK_SIZEOF");
    else if (kind == TK_STRING)
        fprintf(stderr, "TK_STRING");
    else if (kind == TK_CONTINUE)
        fprintf(stderr, "TK_CONTINUE");
    else if (kind == TK_BREAK)
        fprintf(stderr, "TK_BREAK");
    else if (kind == TK_LOGICAL_AND)
        fprintf(stderr, "TK_LOGICAL_AND");
    else if (kind == TK_LOGICAL_OR)
        fprintf(stderr, "TK_LOGICAL_OR");
    else if (kind == TK_ARROW)
        fprintf(stderr, "TK_ARROW");
    else if (kind == TK_TYPEDEF)
        fprintf(stderr, "TK_TYPEDEF");
    else if (kind == TK_LSHIFT)
        fprintf(stderr, "TK_LSHIFT");
    else if (kind == TK_RSHIFT)
        fprintf(stderr, "TK_RSHIFT");
    else if (kind == TK_VARIADIC)
        fprintf(stderr, "TK_VARIADIC");
    else if (kind == TK_EXTERN)
        fprintf(stderr, "TK_EXTERN");
    else if (kind == TK_DO)
        fprintf(stderr, "TK_DO");
    else if (kind == TK_SWITCH)
        fprintf(stderr, "TK_SWITCH");
    else if (kind == TK_CASE)
        fprintf(stderr, "TK_CASE");
    else if (kind == TK_DEFAULT)
        fprintf(stderr, "TK_DEFAULT");
    else if (kind == TK_GOTO)
        fprintf(stderr, "TK_GOTO");
    else if (kind == TK_STATIC)
        fprintf(stderr, "TK_STATIC");
    else if (kind == TK_RESTRICT)
        fprintf(stderr, "TK_RESTRICT");
    else if (kind == TK_REGISTER)
        fprintf(stderr, "TK_REGISTER");
    else if (kind == TK_VOLATILE)
        fprintf(stderr, "TK_VOLATILE");
    else if (kind == TK_AUTO)
        fprintf(stderr, "TK_AUTO");
    else if (kind == TK_ALIGNOF)
        fprintf(stderr, "AK_ALIGNOF");
    else
        fprintf(stderr, "TK_[%c]", kind);

    fprintf(stderr, "\n");
}

void print_type_kind(TypeKind kind) {
    fprintf(stderr, "tokenkind -> ");
    if (kind == TYPE_CHAR)
        fprintf(stderr, "TYPE_CHAR");
    else if (kind == TYPE_SHORT)
        fprintf(stderr, "TYPE_SHORT");
    else if (kind == TYPE_INT)
        fprintf(stderr, "TYPE_INT");
    else if (kind == TYPE_LONG)
        fprintf(stderr, "TYPE_LONG");
    else if (kind == TYPE_PTR)
        fprintf(stderr, "TYPE_PTR");
    else if (kind == TYPE_ARRAY)
        fprintf(stderr, "TYPE_ARRAY");
    else if (kind == TYPE_VOID)
        fprintf(stderr, "TYPE_VOID");
    else if (kind == TYPE_STRUCT)
        fprintf(stderr, "TYPE_STRUCT");
    else if (kind == TYPE_UNION)
        fprintf(stderr, "TYPE_UNION");
    else if (kind == TYPE_ENUM)
        fprintf(stderr, "TYPE_ENUM");
    else if (kind == TYPE_BOOL)
        fprintf(stderr, "TYPE_BOOL");
    else if (kind == TYPE_FUNC)
        fprintf(stderr, "TYPE_FUNC");
    else
        error("print_type_kind() failure: 未定義型 %d", kind);

    fprintf(stderr, "\n");
}

void debug_node(Node *node, char *pos, int depth) {
    if (node == NULL) {
        return;
    }

    printf_with_space(depth, "[%s]\n", pos);

    printf_with_space(depth, "");
    print_node_kind(node->kind);
    puts("");
    debug_type(node->type, depth);

    if (node->kind == ND_NUM) {
        printf_with_space(depth, "");
        fprintf(stderr, "num -> %ld\n", node->val);
    } else if (node->kind == ND_VAR) {
        printf_with_space(depth, "name -> %s\n", node->var->name);
        printf_with_space(depth, "offset -> %d\n", node->var->offset);
    } else if (node->kind == ND_TERNARY) {
        debug_node(node->then, "then", depth + 1);
        debug_node(node->els, "els", depth + 1);
    } else if (node->kind == ND_SUGER || node->kind == ND_BLOCK || node->kind == ND_STMT_EXPR) {
        for (int i = 0; i < node->stmts->len; i++) {
            debug_node(node->stmts->body[i], "stmt", depth + 1);
        }
    } else if (node->kind == ND_SWITCH) {
        debug_node(node->cond, "cond", depth + 1);
        for (int i = 0; i < node->stmts->len; i++) {
            debug_node(node->stmts->body[i], "case-default", depth + 1);
        }
        debug_node(node->body, "body", depth + 1);
    } else if (node->kind == ND_CASE || node->kind == ND_DEFAULT) {
        printf_with_space(depth, "label_name -> %s\n", node->label_name);
        debug_node(node->body, "body", depth + 1);
    } else {
        debug_node(node->lhs, "lhs", depth + 1);
        debug_node(node->rhs, "rhs", depth + 1);
    }
}

void debug_var(Var *var) {
    if (var == NULL) {
        return;
    }

    fprintf(stderr, "name -> %s\n", var->name);
    fprintf(stderr, "len -> %d\n", var->len);
    fprintf(stderr, "offset -> %d\n", var->offset);
    fprintf(stderr, "next_offset -> %d\n", var->next_offset);
}

void debug_initializer(Initializer *init, int depth) {
    if (init == NULL) {
        return;
    }

    if (init->children) {
        debug("[init->children]");
        for (int i = 0; i < init->len; i++) {
            debug_initializer(init->children + i, depth + 1);
        }
        return;
    }

    if (init->expr) {
        debug_node(init->expr, "init->expr", depth);
        return;
    }
}

void debug_type(Type *ty, int depth) {
    if (ty == NULL) {
        return;
    }

    printf_with_space(depth, "");
    print_type_kind(ty->kind);
    puts("");

    if (ty->kind == TYPE_STRUCT || ty->kind == TYPE_UNION) {
        printf_with_space(depth, "name -> %s\n", ty->name);
        printf_with_space(depth, "size -> %d\n", ty->size);
        for (Var *member = ty->member; member; member = member->next) {
            printf_with_space(depth, "member -> %s\n", member->name);
        }
    } else {
        printf_with_space(depth, "size -> %d\n", ty->size);
        printf_with_space(depth, "array_size -> %d\n", ty->array_size);
        printf_with_space(depth, "is_constant -> %d\n", ty->is_constant);
        debug_type(ty->ptr_to, depth + 1);
    }
}

void debug_token(Token *t) {
    if (t == NULL) {
        return;
    }

    print_token_kind(t->kind);
    debug_type(t->type, 0);
    fprintf(stderr, "val -> %ld\n", t->val);
    fprintf(stderr, "str -> %.10s\n", t->str);
    fprintf(stderr, "str_literal_index -> %d\n", t->str_literal_index);
    fprintf(stderr, "len; -> %d\n", t->len);
}

void debug(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}
