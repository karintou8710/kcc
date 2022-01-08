#include "9cc.h"

void print_node_kind(NodeKind kind)
{
    fprintf(stderr, "nodekind -> ");
    if (kind == ND_ADD)
        fprintf(stderr, "ND_ADD"); // +
    else if (kind == ND_SUB)
        fprintf(stderr, "ND_SUB"); // -
    else if (kind == ND_MUL)
        fprintf(stderr, "ND_MUL"); // *
    else if (kind == ND_DIV)
        fprintf(stderr, "ND_DIV"); // /
    else if (kind == ND_MOD)
        fprintf(stderr, "ND_MOD"); // %
    else if (kind == ND_ASSIGN)
        fprintf(stderr, "ND_ASSIGN"); // =
    else if (kind == ND_EQ)
        fprintf(stderr, "ND_EQ"); // ==
    else if (kind == ND_NE)
        fprintf(stderr, "ND_NE"); // !=
    else if (kind == ND_LT)
        fprintf(stderr, "ND_LT"); // <
    else if (kind == ND_LE)
        fprintf(stderr, "ND_LE"); // <=
    else if (kind == ND_VAR)
        fprintf(stderr, "ND_VAR"); // local var
    else if (kind == ND_NUM)
        fprintf(stderr, "ND_NUM"); // num
    else if (kind == ND_RETURN)
        fprintf(stderr, "ND_RETURN"); // return
    else if (kind == ND_IF)
        fprintf(stderr, "ND_IF"); // if
    else if (kind == ND_ELSE)
        fprintf(stderr, "ND_ELSE"); // else
    else if (kind == ND_FOR)
        fprintf(stderr, "ND_FOR"); // for
    else if (kind == ND_WHILE)
        fprintf(stderr, "ND_WHILE"); // while
    else if (kind == ND_BLOCK)
        fprintf(stderr, "ND_BLOCK"); // block {}
    else if (kind == ND_CALL)
        fprintf(stderr, "ND_CALL"); // call
    else if (kind == ND_ADDR)
        fprintf(stderr, "ND_ADDR"); // & アドレス
    else if (kind == ND_DEREF)
        fprintf(stderr, "ND_DEREF"); // * ポインタ
    else if (kind == ND_STRING)
        fprintf(stderr, "ND_STRING"); // string literal
    else if (kind == ND_CONTINUE)
        fprintf(stderr, "ND_CONTINUE"); // continue
    else if (kind == ND_BREAK)
        fprintf(stderr, "ND_BREAK"); // break
    
    fprintf(stderr, "\n");
}

void debug_node(Node *node, char *pos)
{
    fprintf(stderr, "pos %s\n", pos);

    if (node == NULL)
    {
        fprintf(stderr, "node -> NULL\n");
        return;
    }

    print_node_kind(node->kind);

    debug_node(node->lhs, "lhs");
    debug_node(node->rhs, "rhs");
    // debug_var(node->var);
    // debug_type(node->type);
}

void debug_var(Var *var)
{
    if (var == NULL)
    {
        fprintf(stderr, "var -> NULL\n");
        return;
    }

    fprintf(stderr, "name -> %s\n", var->name);
    fprintf(stderr, "len -> %d\n", var->len);
    fprintf(stderr, "offset -> %d\n", var->offset);

    debug_type(var->type);
}

void debug_type(Type *ty)
{
    if (ty == NULL)
    {
        fprintf(stderr, "type -> NULL\n");
        return;
    }

    fprintf(stderr, "typekind -> %d\n", ty->kind);
    fprintf(stderr, "size -> %d\n", ty->size);
    fprintf(stderr, "array_size -> %d\n", ty->array_size);
    fprintf(stderr, "\n");

    debug_type(ty->ptr_to);
}
