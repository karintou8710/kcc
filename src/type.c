#include "9cc.h"

/*
 * TODO: 型のキャスト、静的型チェック
 *
 */

/* 固定長の型のサイズを返す */
static int tykind_to_size(TypeKind tykind) {
    if (tykind == TYPE_VOID) {
        return 0;
    } else if (tykind == TYPE_CHAR) {
        return 1;
    } else if (tykind == TYPE_INT) {
        return 4;
    } else if (tykind == TYPE_PTR) {
        return 8;
    } else if (tykind == TYPE_STRUCT) {
        return 0;  // parser側でsizeを決める
    } else if (tykind == TYPE_ENUM) {
        return 4;  // INT型と同じサイズ
    }

    error("存在しないまたは固定長ではない型です");
}

int array_base_type_size(Type *ty) {
    if (ty->kind == TYPE_ARRAY) {
        return array_base_type_size(ty->ptr_to);
    }

    return ty->size;
}

// sizeofの実装
int sizeOfType(Type *ty) {
    return ty->size;
}

bool is_same_type(Type *ty1, Type *ty2) {
    if (ty1 == NULL || ty2 == NULL) {
        // NULL == NULLで等しい型
        return ty1 == ty2;
    }
    if (ty1->kind != ty2->kind) {
        return false;
    }

    return is_same_type(ty1->ptr_to, ty2->ptr_to);
}

/* 基本の型を生成 */
Type *new_type(TypeKind tykind) {
    Type *ty = memory_alloc(sizeof(Type));
    ty->kind = tykind;
    ty->size = tykind_to_size(tykind);
    return ty;
}

/* ポインター型を生成 */
Type *new_ptr_type(Type *ptr_to) {
    Type *ty = memory_alloc(sizeof(Type));
    ty->kind = TYPE_PTR;
    ty->size = tykind_to_size(TYPE_PTR);
    ty->ptr_to = ptr_to;
    return ty;
}

// TODO: baseの型(例えばint[2][3]ならint型)を全ての配列型で持つべき?
/* 配列型を生成 */
Type *new_array_type(Type *ptr_to, int array_size) {
    Type *ty = memory_alloc(sizeof(Type));
    ty->kind = TYPE_ARRAY;
    ty->size = ptr_to->size * array_size;
    ty->array_size = array_size;
    ty->ptr_to = ptr_to;
    return ty;
}

bool is_integertype(TypeKind kind) {
    return (
        kind == TYPE_CHAR ||
        kind == TYPE_INT ||
        kind == TYPE_ENUM);
}

bool is_scalartype(TypeKind kind) {
    return (
        is_integertype(kind) ||
        kind == TYPE_PTR);
}

bool is_relationalnode(NodeKind kind) {
    return (
        kind == ND_EQ ||
        kind == ND_NE ||
        kind == ND_LT ||
        kind == ND_LE);
}

TypeKind large_numtype(Type *t1, Type *t2) {
    if (!is_integertype(t1->kind) || !is_integertype(t2->kind)) {
        error("整数の型ではありません。\n");
    }

    if (t1->size >= t2->size) {
        return t1->kind;
    } else {
        return t2->kind;
    }
}

/* キャスト */
bool can_type_cast(Type *ty, TypeKind to) {
    TypeKind from = ty->kind;

    if (to == TYPE_VOID) {
        return true;
    }

    if (!is_scalartype(from) || !is_scalartype(to)) {
        return false;
    }

    return true;
}

/*
 * 必要なノードが型を持つことを保証するようにする
 * 演算子の前にadd_typeが必要
 * ND_NUM, ND_CALL, ND_VAR, ND_ADD,SUB,MUL,DIV,
 */
void add_type(Node *node) {
    if (node->type != NULL)
        return;

    if (node->kind == ND_VAR) {
        node->type = node->var->type;
        return;
    }

    if (node->kind == ND_ADDR) {
        node->type = new_ptr_type(node->lhs->type);
        return;
    }

    if (node->kind == ND_NOT) {
        node->type = node->lhs->type;
        return;
    }

    if (node->kind == ND_DEREF) {
        Type *ty = node->lhs->type;
        if (!ty->ptr_to) {
            error("add_type() failure: derefに失敗しました");
        }
        node->type = ty->ptr_to;
        return;
    }

    if (node->kind == ND_TERNARY) {
        if (is_integertype(node->then->type->kind) && is_integertype(node->els->type->kind)) {
            node->type = new_type(large_numtype(node->then->type, node->els->type));
            return;
        }

        // TODO: 構造体と配列
        if (node->then->type->kind == TYPE_STRUCT && node->els->type->kind == TYPE_STRUCT) {
            if (node->then->type != node->els->type) {
                error("add_type() failure: ND_TERNARY, different struct type");
            }
            node->type = node->then->type;
            return;
        }

        error("add_type() failure: ND_TERNARY error.");
        return;
    }

    if (node->kind == ND_NUM) {
        node->type = new_type(TYPE_INT);
        return;
    }

    if (node->kind == ND_STRING) {
        node->type = new_ptr_type(new_type(TYPE_CHAR));
        return;
    }

    if (node->kind == ND_CALL) {
        Function *fn = find_func(node->fn_name);
        if (!fn) {
            // TODO: プロトタイプ宣言に対応
            node->type = new_type(TYPE_INT);
            // error("add_type() failure: can't find func, name=%s", node->fn_name);
        } else {
            node->type = fn->ret_type;
        }

        return;
    }

    if (node->kind == ND_ASSIGN) {
        if (node->rhs->type == NULL || node->lhs->type == NULL) {
            fprintf(stderr, "[node->lhs->type]\n");
            debug_type(node->lhs->type, 0);
            fprintf(stderr, "[node->rhs->type]\n");
            debug_type(node->rhs->type, 0);
            debug_node(node->rhs, "root", 0);
            error("add_type() failure: type not found(ND_ASSIGN)");
        }

        if (can_type_cast(node->rhs->type, node->lhs->type->kind)) {
            node->type = new_type(node->lhs->type->kind);
            return;
        }

        // 配列は暗黙にポインターとして扱う
        if (node->rhs->type->kind == TYPE_ARRAY && node->lhs->type->kind == TYPE_PTR) {
            node->type = new_type(node->lhs->type->kind);
            return;
        }

        fprintf(stderr, "[node->lhs->type]\n");
        debug_type(node->lhs->type, 0);
        fprintf(stderr, "[node->rhs->type]\n");
        debug_type(node->rhs->type, 0);
        error("add_type() failure: fail to cast %d -> %d", node->rhs->type->kind, node->lhs->type->kind);
    }

    // lhsとrhsの順番はparse側で保証する
    if (node->kind == ND_ADD) {
        Node *lhs = node->lhs, *rhs = node->rhs;
        if (is_integertype(lhs->type->kind) && is_integertype(rhs->type->kind)) {
            node->type = new_type(large_numtype(lhs->type, rhs->type));
            return;
        }

        if (is_integertype(lhs->type->kind) && rhs->type->kind == TYPE_PTR) {
            node->type = rhs->type;
            return;
        }

        if (is_integertype(lhs->type->kind) && rhs->type->kind == TYPE_ARRAY) {
            node->type = rhs->type;
            return;
        }

        error("%d %d不正な型です(ADD)", lhs->type->kind, rhs->type->kind);
    }

    if (node->kind == ND_SUB) {
        Node *lhs = node->lhs, *rhs = node->rhs;

        if (is_integertype(lhs->type->kind) && is_integertype(rhs->type->kind)) {
            node->type = new_type(large_numtype(lhs->type, rhs->type));
            return;
        }

        if (lhs->type->kind == TYPE_PTR && is_integertype(rhs->type->kind)) {
            node->type = rhs->type;
            return;
        }

        if (lhs->type->kind == TYPE_ARRAY && is_integertype(rhs->type->kind)) {
            node->type = rhs->type;
            return;
        }

        if (lhs->type->kind == TYPE_PTR && rhs->type->kind == TYPE_PTR ||
            lhs->type->kind == TYPE_ARRAY && rhs->type->kind == TYPE_ARRAY ||
            lhs->type->kind == TYPE_ARRAY && rhs->type->kind == TYPE_PTR ||
            lhs->type->kind == TYPE_PTR && rhs->type->kind == TYPE_ARRAY) {
            node->type = new_type(TYPE_INT);  // gccではlong
            return;
        }

        error("%d %d不正な型です(SUB)", lhs->type->kind, rhs->type->kind);
    }

    if (node->kind == ND_MUL) {
        Node *lhs = node->lhs, *rhs = node->rhs;
        if (is_integertype(lhs->type->kind) && is_integertype(rhs->type->kind)) {
            node->type = new_type(TYPE_INT);
            return;
        }

        error("%d %d不正な型です(MUL)", lhs->type->kind, rhs->type->kind);
    }

    if (node->kind == ND_DIV) {
        Node *lhs = node->lhs, *rhs = node->rhs;
        if (is_integertype(lhs->type->kind) && is_integertype(rhs->type->kind)) {
            node->type = new_type(TYPE_INT);
            return;
        }

        error("%d %d不正な型です(DIV)", lhs->type->kind, rhs->type->kind);
    }

    if (node->kind == ND_MOD) {
        Node *lhs = node->lhs, *rhs = node->rhs;
        if (is_integertype(lhs->type->kind) && is_integertype(rhs->type->kind)) {
            node->type = new_type(TYPE_INT);
            return;
        }

        error("%d %d不正な型です(DIV)", lhs->type->kind, rhs->type->kind);
    }

    if (node->kind == ND_LOGICALNOT || node->kind == ND_LOGICAL_AND || node->kind == ND_LOGICAL_OR) {
        node->type = new_type(TYPE_INT);
        return;
    }

    if (node->kind == ND_AND || node->kind == ND_OR || node->kind == ND_XOR) {
        Node *lhs = node->lhs, *rhs = node->rhs;
        if (is_integertype(lhs->type->kind) && is_integertype(rhs->type->kind)) {
            node->type = new_type(large_numtype(lhs->type, rhs->type));
            return;
        }

        error("%d %d不正な型です(ADD)", lhs->type->kind, rhs->type->kind);
    }

    if (is_relationalnode(node->kind)) {
        node->type = new_type(TYPE_INT);
        return;
    }

    debug_node(node, "root", 0);
    error("add_type() failure: 対応していないノードタイプです。");
}