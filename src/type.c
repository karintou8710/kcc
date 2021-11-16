#include "9cc.h"

/*
 * TODO: 型のキャスト、静的型チェック
 *
 */

/* 固定長の型のサイズを返す */
static int tykind_to_size(TypeKind tykind)
{
    switch (tykind)
    {
    case TYPE_CHAR:
        return 1;
        break;
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

int is_numtype(TypeKind kind) {
    return (
        kind == TYPE_CHAR ||
        kind == TYPE_INT
    );
}

TypeKind large_numtype(Type *t1, Type *t2) {
    if (!is_numtype(t1->kind) || !is_numtype(t2->kind)) {
        error("整数の型ではありません。\n");
    }

    if (t1->size >= t2->size) {
        return t1->kind;
    } else {
        return t2->kind;
    }
}

/* キャスト */

/*
 * 必要なノードが型を持つことを保証するようにする
 * 演算子の前にadd_typeが必要
 * ND_NUM, ND_CALL, ND_VAR, ND_ADD,SUB,MUL,DIV,
 */
void add_type(Node *node)
{

    if (node->type != NULL)
        return;

    if (node->kind == ND_VAR)
    {
        node->type = node->var->type;
        return;
    }

    if (node->kind == ND_DEREF)
    {
        Type *ty = node->lhs->type;
        if (!ty->ptr_to)
        {
            error("derefに失敗しました");
        }
        node->type = ty->ptr_to;
        return;
    }

    if (node->kind == ND_NUM)
    {
        node->type = new_type(TYPE_INT);
        return;
    }

    if (node->kind == ND_STRING)
    {
        node->type = new_ptr_type(new_type(TYPE_CHAR));
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
        if (is_numtype(lhs->type->kind) && is_numtype(rhs->type->kind))
        {
            node->type = new_type(large_numtype(lhs->type, rhs->type));
            return;
        }

        if (is_numtype(lhs->type->kind) && rhs->type->kind == TYPE_PTR)
        {
            node->type = rhs->type;
            return;
        }

        if (is_numtype(lhs->type->kind) && rhs->type->kind == TYPE_ARRAY)
        {
            node->type = rhs->type;
            return;
        }

        error("%d %d不正な型です(ADD)", lhs->type->kind, rhs->type->kind);
    }

    if (node->kind == ND_SUB)
    {
        Node *lhs = node->lhs, *rhs = node->rhs;

        if (is_numtype(lhs->type->kind) && is_numtype(rhs->type->kind))
        {
            node->type = new_type(large_numtype(lhs->type, rhs->type));
            return;
        }

        if (lhs->type->kind == TYPE_PTR && is_numtype(lhs->type->kind))
        {
            node->type = rhs->type;
            return;
        }

        if (lhs->type->kind == TYPE_ARRAY && is_numtype(lhs->type->kind))
        {
            node->type = rhs->type;
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

    if (node->kind == ND_MOD)
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