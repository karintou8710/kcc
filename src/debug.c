#include "9cc.h"

void recursion_line_printf(int depth, char *fmt, ...) {
    va_list ap;
    int space_width = 4;
    va_start(ap, fmt);
    fprintf(stderr, "%*s", depth * space_width, "");
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
    else if (kind == ND_LOGICALNOT)
        fprintf(stderr, "ND_LOGICALNOT");  // !
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
    else
        error("print_node_kind() failure");

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
    else if (kind == TK_INC)
        fprintf(stderr, "TK_INC");
    else if (kind == TK_DEC)
        fprintf(stderr, "TK_DEC");
    else if (kind == TK_DIV_EQ)
        fprintf(stderr, "TK_DIV_EQ");
    else if (kind == TK_MOD_EQ)
        fprintf(stderr, "TK_MOD_EQ");
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
    else
        fprintf(stderr, "TK_[%c]", kind);

    fprintf(stderr, "\n");
}

void print_type_kind(TypeKind kind) {
    fprintf(stderr, "tokenkind -> ");
    if (kind == TYPE_CHAR)
        fprintf(stderr, "TYPE_CHAR");
    else if (kind == TYPE_INT)
        fprintf(stderr, "TYPE_INT");
    else if (kind == TYPE_PTR)
        fprintf(stderr, "TYPE_PTR");
    else if (kind == TYPE_ARRAY)
        fprintf(stderr, "TYPE_ARRAY");
    else if (kind == TYPE_VOID)
        fprintf(stderr, "TYPE_VOID");
    else if (kind == TYPE_STRUCT)
        fprintf(stderr, "TYPE_STRUCT");
    else if (kind == TYPE_ENUM)
        fprintf(stderr, "TYPE_ENUM");
    else
        error("print_type_kind() failure: unexpected type %d", kind);

    fprintf(stderr, "\n");
}

void debug_node(Node *node, char *pos, int depth) {
    if (node == NULL) {
        return;
    }

    recursion_line_printf(depth, "[%s]\n", pos);

    recursion_line_printf(depth, "");
    print_node_kind(node->kind);
    puts("");

    if (node->kind == ND_NUM) {
        recursion_line_printf(depth, "");
        fprintf(stderr, "num -> %d\n", node->val);
    } else if (node->kind == ND_VAR) {
        recursion_line_printf(depth, "name -> %s\n", node->var->name);
        recursion_line_printf(depth, "offset -> %d\n", node->var->offset);
    } else if (node->kind == ND_TERNARY) {
        debug_node(node->then, "then", depth + 1);
        debug_node(node->els, "els", depth + 1);
    } else if (node->kind == ND_SUGER || node->kind == ND_BLOCK) {
        for (int i = 0; i < node->stmts->len; i++) {
            debug_node(node->stmts->body[i], "stmt", depth + 1);
        }
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

    recursion_line_printf(depth, "");
    print_type_kind(ty->kind);
    puts("");

    if (ty->kind == TYPE_STRUCT) {
        recursion_line_printf(depth, "name -> %s\n", ty->name);
        recursion_line_printf(depth, "size -> %d\n", ty->size);
        for (Var *member = ty->member; member; member = member->next) {
            recursion_line_printf(depth, "member -> %s\n", member->name);
        }
    } else {
        recursion_line_printf(depth, "size -> %d\n", ty->size);
        recursion_line_printf(depth, "array_size -> %d\n", ty->array_size);
        debug_type(ty->ptr_to, depth + 1);
    }
}

void debug_token(Token *t) {
    if (t == NULL) {
        return;
    }

    print_token_kind(t->kind);
    debug_type(t->type, 0);
    fprintf(stderr, "val -> %d\n", t->val);
    fprintf(stderr, "str -> %s\n", t->str);
    fprintf(stderr, "str_literal_index -> %d\n", t->str_literal_index);
    fprintf(stderr, "len; -> %d\n", t->len);
}

void debug(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}
