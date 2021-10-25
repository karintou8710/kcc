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