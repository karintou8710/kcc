#include "9cc.h"

static int tykind_to_size(TypeKind tykind) {
    switch (tykind) {
        case TYPE_INT:
            return 4;
            break;
        case TYPE_PTR:
            return 8;
            break;
    }

    error("存在しない型です");
}

Type *new_type(TypeKind tykind) {
    Type *ty = calloc(1, sizeof(Type));
    ty->kind = tykind;
    ty->size = tykind_to_size(tykind);
    return ty;
}

Type *new_ptr_type(Type *ptr_to) {
    Type *ty = calloc(1, sizeof(Type));
    ty->kind = TYPE_PTR;
    ty->size = tykind_to_size(TYPE_PTR);
    ty->ptr_to = ptr_to;
    return ty;
}

/*
 * ND_NUM, ND_CALL, ND_LVAR, ND_ADD,SUB,MUL,DIV, 
 *
 */ 
void add_type(Node *node) {

    if (node->type != NULL) return;

    if (node->kind == ND_LVAR || node->kind == ND_DEREF) {
        node->type = node->lvar->type;
        return;
    }

    if (node->kind == ND_NUM) {
        node->type = new_type(TYPE_INT);
        return;
    }

    if (node->kind == ND_CALL) {
        /* TODO: とりあえずINT型だけど、関数の戻り値の型に直す */
        node->type = new_type(TYPE_INT);
        return;
    }

    if (node->kind == ND_ADD) {
        Node *lhs = node->lhs, *rhs = node->rhs;
        if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_INT) {
            node->type = new_type(TYPE_INT);
        }

        if (lhs->type->kind == TYPE_PTR && rhs->type->kind == TYPE_INT) {
            node->type = new_type(TYPE_PTR);
        }

        if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_PTR) {
            node->type = new_type(TYPE_PTR);
        }
    }

    if (node->kind == ND_SUB) {
        Node *lhs = node->lhs, *rhs = node->rhs;
        if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_INT) {
            node->type = new_type(TYPE_INT);
        }

        if (lhs->type->kind == TYPE_PTR && rhs->type->kind == TYPE_INT) {
            node->type = new_type(TYPE_PTR);
        }

        if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_PTR) {
            node->type = new_type(TYPE_PTR);
        }
    }

    if (node->kind == ND_MUL) {
        Node *lhs = node->lhs, *rhs = node->rhs;
        if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_INT) {
            node->type = new_type(TYPE_INT);
        }
    }

    if (node->kind == ND_DIV) {
        Node *lhs = node->lhs, *rhs = node->rhs;
        if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_INT) {
            node->type = new_type(TYPE_INT);
        }
    }

}