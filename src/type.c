#include "kcc.h"

/* 固定長の型のサイズを返す */
static int typekind_to_size(TypeKind tykind) {
    if (tykind == TYPE_VOID) {
        return 0;
    } else if (tykind == TYPE_CHAR || tykind == TYPE_BOOL) {
        return 1;
    } else if (tykind == TYPE_SHORT) {
        return 2;
    } else if (tykind == TYPE_INT) {
        return 4;
    } else if (tykind == TYPE_PTR || tykind == TYPE_LONG) {
        return 8;
    } else if (tykind == TYPE_STRUCT || tykind == TYPE_UNION) {
        return 0;  // parser側でsizeを決める
    } else if (tykind == TYPE_ENUM) {
        return 4;  // INT型と同じサイズ
    } else if (tykind == TYPE_FUNC) {
        return 0;  // 関数型にサイズは存在しない
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
int sizeof_type(Type *ty) {
    return ty->size;
}

size_t alignof_type(Type *ty) {
    return ty->alignment;
}

void apply_align_to_struct(Type *ty) {
    if (!ty || (ty->kind != TYPE_STRUCT && ty->kind != TYPE_UNION)) {
        error("apply_align_to_struct() failure: struct or union型ではありません");
    }

    if (ty->member == NULL) return;

    size_t max_alginemnt = 0;
    for (Var *v = ty->member; v; v = v->next) {
        size_t alignment = v->type->alignment;
        if (max_alginemnt < alignment) max_alginemnt = alignment;
    }

    ty->alignment = max_alginemnt;

    Var *v = ty->member;
    v->offset = 0;
    int struct_size = 0;

    while (v->next) {
        if (ty->kind == TYPE_UNION) {
            v->next->offset = 0;
            if (struct_size < sizeof_type(v->type)) struct_size = sizeof_type(v->type);
        } else {
            // struct
            v->next->offset = v->offset + sizeof_type(v->type);
            // 負の割り算は未実装
            int padding = (alignof_type(v->next->type) - v->next->offset % alignof_type(v->next->type)) % alignof_type(v->next->type);
            v->next->offset += padding;
            struct_size += sizeof_type(v->type) + padding;
        }
        v = v->next;
    }
    // 最後の一つのメンバーのサイズを足す
    if (ty->kind == TYPE_UNION) {
        if (struct_size < sizeof_type(v->type)) struct_size = sizeof_type(v->type);
    } else {
        struct_size += sizeof_type(v->type);
    }

    // max_alignmentの倍数に構造体のサイズを揃える
    int padding = (max_alginemnt - struct_size % max_alginemnt) % max_alginemnt;
    struct_size += padding;

    ty->size = struct_size;
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

/* tokenはコピー対象に含めない */
void shallowcopy_type(Type *to, Type *from) {
    if (from == NULL) {
        return;
    }
    to->kind = from->kind;
    to->ptr_to = try_memory_allocation(sizeof(Type));
    to->ptr_to = from->ptr_to;
    to->size = from->size;
    to->array_size = from->array_size;
    to->alignment = from->alignment;
    to->name = from->name;
    to->member = from->member;
    to->is_forward = from->is_forward;
    to->is_constant = from->is_constant;
    to->is_unsigned = from->is_unsigned;
    return;
}

void copy_type(Type *to, Type *from) {
    if (from == NULL) {
        return;
    }
    to->kind = from->kind;
    to->ptr_to = try_memory_allocation(sizeof(Type));
    copy_type(to->ptr_to, from->ptr_to);
    to->size = from->size;
    to->array_size = from->array_size;
    to->alignment = from->alignment;
    to->name = from->name;
    to->member = from->member;
    to->is_forward = from->is_forward;
    to->is_constant = from->is_constant;
    to->is_unsigned = from->is_unsigned;
    return;
}

/* ネスト型は最後まで配列型のサイズが決定しないので計算し直す */
void calc_type_size(Type *type) {
    if (type == NULL || type->ptr_to == NULL) return;

    calc_type_size(type->ptr_to);
    if (type->kind == TYPE_ARRAY) {
        type->size = type->array_size * type->ptr_to->size;
    }
}

/* 基本の型を生成 */
Type *new_type(TypeKind tykind) {
    Type *ty = try_memory_allocation(sizeof(Type));
    ty->kind = tykind;
    ty->size = typekind_to_size(tykind);
    ty->alignment = ty->size;  // 基本型のアライメントはサイズと等しい
    return ty;
}

/* ポインター型を生成 */
Type *new_ptr_type(Type *ptr_to) {
    Type *ty = try_memory_allocation(sizeof(Type));
    ty->kind = TYPE_PTR;
    ty->size = typekind_to_size(TYPE_PTR);
    ty->alignment = ty->size;
    ty->ptr_to = ptr_to;
    return ty;
}

/* 配列型を生成 */
Type *new_array_type(Type *ptr_to, int array_size) {
    Type *ty = try_memory_allocation(sizeof(Type));
    ty->kind = TYPE_ARRAY;
    ty->size = ptr_to->size * array_size;
    ty->alignment = ptr_to->alignment;
    ty->array_size = array_size;
    ty->ptr_to = ptr_to;
    return ty;
}

Type *new_func_type(Type *ret_type, Var *params, bool *is_variadic, char *name) {
    // strcmp()を使用するので、nameをNULLにしない
    if (name == NULL) name = "";
    Type *type = new_type(TYPE_FUNC);
    type->ptr_to = ret_type;
    type->params = params;
    type->is_variadic = is_variadic;
    type->name = name;
    return type;
}

/* base_typeは型を独立させるためコピーする */
Tag *new_tag(Type *type) {
    Tag *tag = try_memory_allocation(sizeof(Tag));
    tag->base_type = try_memory_allocation(sizeof(Type));
    copy_type(tag->base_type, type);
    tag->forward_type = new_vec();
    return tag;
}

bool is_integertype(TypeKind kind) {
    return (
        kind == TYPE_CHAR ||
        kind == TYPE_SHORT ||
        kind == TYPE_INT ||
        kind == TYPE_LONG ||
        kind == TYPE_ENUM ||
        kind == TYPE_BOOL);
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

TypeKind large_integer_type(Type *t1, Type *t2) {
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
bool can_cast_type(Type *ty, TypeKind to) {
    TypeKind from = ty->kind;

    if (to == TYPE_VOID) {
        return true;
    }

    if (is_scalartype(from) && is_scalartype(to)) {
        return true;
    }

    if ((from == TYPE_ARRAY || from == TYPE_FUNC) && is_scalartype(to)) {
        return true;
    }

    return false;
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
        Type *then_t = node->then->type, *els_t = node->els->type;

        // 算術型
        if (is_integertype(then_t->kind) && is_integertype(els_t->kind)) {
            node->type = new_type(large_integer_type(then_t, els_t));
            return;
        }

        // 示す先が同じポインター
        if (then_t->kind == TYPE_PTR && els_t->kind == TYPE_PTR) {
            if (!is_same_type(then_t, els_t)) {
                error("add_type() failure: ND_TERNARY, different pointer type");
            }
            node->type = then_t;
            return;
        }

        // TODO: 構造体と配列
        if (then_t->kind == TYPE_STRUCT && els_t->kind == TYPE_STRUCT) {
            if (then_t != els_t) {
                error("add_type() failure: ND_TERNARY, different struct type");
            }
            node->type = then_t;
            return;
        }

        if (then_t->kind == TYPE_UNION && els_t->kind == TYPE_UNION) {
            if (then_t != els_t) {
                error("add_type() failure: ND_TERNARY, different union type");
            }
            node->type = then_t;
            return;
        }

        error("add_type() failure: %d %d 不正な型です(ND_TERNARY)", then_t->kind, els_t->kind);
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
        if (node->lhs) {
            if (!(node->lhs->type->kind == TYPE_PTR && node->lhs->type->ptr_to->kind == TYPE_FUNC)) {
                error("add_type() failure: 関数ポインターではありません(ND_CALL)");
            }
            node->type = node->lhs->type->ptr_to->ptr_to;
        } else {
            Function *fn = find_func(node->fn_name);
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

        if (can_cast_type(node->rhs->type, node->lhs->type->kind)) {
            node->type = node->lhs->type;
            return;
        }

        // 配列は暗黙にポインターとして扱う
        if (node->rhs->type->kind == TYPE_ARRAY && node->lhs->type->kind == TYPE_PTR) {
            node->type = new_type(node->lhs->type->kind);
            return;
        }

        if (node->rhs->type->kind == TYPE_STRUCT && node->lhs->type->kind == TYPE_STRUCT) {
            if (node->rhs->type != node->lhs->type) {
                error("add_type() failure: 異なる型の構造体に代入はできません");
            }

            node->type = node->rhs->type;
            return;
        }

        if (node->rhs->type->kind == TYPE_UNION && node->lhs->type->kind == TYPE_UNION) {
            if (node->rhs->type != node->lhs->type) {
                error("add_type() failure: 異なる型の共用体に代入はできません");
            }

            node->type = node->rhs->type;
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
            node->type = new_type(large_integer_type(lhs->type, rhs->type));
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
            node->type = new_type(large_integer_type(lhs->type, rhs->type));
            return;
        }

        if (lhs->type->kind == TYPE_PTR && is_integertype(rhs->type->kind)) {
            node->type = lhs->type;
            return;
        }

        if (lhs->type->kind == TYPE_ARRAY && is_integertype(rhs->type->kind)) {
            node->type = lhs->type;
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

        error("%d %d不正な型です(MOD)", lhs->type->kind, rhs->type->kind);
    }

    if (node->kind == ND_LOGICAL_NOT || node->kind == ND_LOGICAL_AND || node->kind == ND_LOGICAL_OR) {
        node->type = new_type(TYPE_INT);
        return;
    }

    if (node->kind == ND_AND ||
        node->kind == ND_OR ||
        node->kind == ND_XOR ||
        node->kind == ND_LSHIFT ||
        node->kind == ND_RSHIFT) {
        Node *lhs = node->lhs, *rhs = node->rhs;
        if (is_integertype(lhs->type->kind) && is_integertype(rhs->type->kind)) {
            node->type = new_type(large_integer_type(lhs->type, rhs->type));
            return;
        }

        error("%d %d不正な型です", lhs->type->kind, rhs->type->kind);
    }

    if (is_relationalnode(node->kind)) {
        node->type = new_type(TYPE_INT);
        return;
    }

    if (node->kind == ND_SUGER || node->kind == ND_STMT_EXPR) {
        Node *n = vec_last(node->stmts);
        add_type(n);
        node->type = n->type;
        return;
    }

    error("add_type() failure: 対応していないノードタイプです。");
}