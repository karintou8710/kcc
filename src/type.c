#include "9cc.h"

/* 固定長の型のサイズを返す */
static int tykind_to_size(TypeKind tykind)
{
    switch (tykind)
    {
    case TYPE_INT:
        return 4;
        break;
    case TYPE_PTR:
        return 8;
        break;
    }

    error("存在しないまたは固定長ではない型です");
}

// sizeofの実装
int sizeOfType(Type *ty)
{
    return ty->size;
}

/* 基本の型を生成 */
Type *new_type(TypeKind tykind)
{
    Type *ty = calloc(1, sizeof(Type));
    ty->kind = tykind;
    ty->size = tykind_to_size(tykind);
    return ty;
}

/* ポインター型を生成 */
Type *new_ptr_type(Type *ptr_to)
{
    Type *ty = calloc(1, sizeof(Type));
    ty->kind = TYPE_PTR;
    ty->size = tykind_to_size(TYPE_PTR);
    ty->ptr_to = ptr_to;
    return ty;
}

// TODO: baseの型(例えばint[2][3]ならint型)を全ての配列型で持つべき?
/* 配列型を生成 */
Type *new_array_type(Type *ptr_to, int array_size)
{
    Type *ty = calloc(1, sizeof(Type));
    ty->kind = TYPE_ARRAY;
    ty->size = ptr_to->size * array_size;
    ty->array_size = array_size;
    ty->ptr_to = ptr_to;
    return ty;
}

/* キャスト */

/* 配列型からポインターへキャスト */
Type *cast_array_to_ptr(Type *ptr)
{
    while (ptr->ptr_to)
        ptr = ptr->ptr_to;
    Type *ty = new_ptr_type(new_type(ptr->kind));
    return ty;
}


/*
 * 必要なノードが型を持つことを保証するようにする
 * 演算子の前にadd_typeが必要
 * ND_NUM, ND_CALL, ND_LVAR, ND_ADD,SUB,MUL,DIV,
 */
void add_type(Node *node)
{

    if (node->type != NULL)
        return;

    if (node->kind == ND_LVAR || node->kind == ND_DEREF)
    {
        node->type = node->lvar->type;
        return;
    }

    if (node->kind == ND_NUM)
    {
        node->type = new_type(TYPE_INT);
        return;
    }

    if (node->kind == ND_CALL)
    {
        /* TODO: とりあえずINT型だけど、関数の戻り値の型に直す */
        node->type = new_type(TYPE_INT);
        return;
    }

    // lhsとrhsの順番はparse側で保証する
    if (node->kind == ND_ADD)
    {
        Node *lhs = node->lhs, *rhs = node->rhs;
        if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_INT)
        {
            node->type = new_type(TYPE_INT);
            return;
        }

        if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_PTR)
        {
            node->type = rhs->type;
            return;
        }

        if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_ARRAY)
        {
            node->type = cast_array_to_ptr(rhs->type); // 暗黙にポインター型にキャスト
            return;
        }

        error("%d %d不正な型です(ADD)", lhs->type->kind, rhs->type->kind);
    }

    if (node->kind == ND_SUB)
    {
        Node *lhs = node->lhs, *rhs = node->rhs;
        if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_INT)
        {
            node->type = new_type(TYPE_INT);
            return;
        }

        if (lhs->type->kind == TYPE_PTR && rhs->type->kind == TYPE_INT)
        {
            node->type = rhs->type;
            return;
        }

        if (lhs->type->kind == TYPE_ARRAY && rhs->type->kind == TYPE_INT)
        {
            node->type = cast_array_to_ptr(rhs->type); // 暗黙にポインター型にキャスト
            return;
        }

        error("%d %d不正な型です(SUB)", lhs->type->kind, rhs->type->kind);
    }

    if (node->kind == ND_MUL)
    {
        Node *lhs = node->lhs, *rhs = node->rhs;
        if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_INT)
        {
            node->type = new_type(TYPE_INT);
            return;
        }

        error("%d %d不正な型です(MUL)", lhs->type->kind, rhs->type->kind);
    }

    if (node->kind == ND_DIV)
    {
        Node *lhs = node->lhs, *rhs = node->rhs;
        if (lhs->type->kind == TYPE_INT && rhs->type->kind == TYPE_INT)
        {
            node->type = new_type(TYPE_INT);
            return;
        }

        error("%d %d不正な型です(DIV)", lhs->type->kind, rhs->type->kind);
    }
}