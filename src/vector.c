#include "kcc.h"

Vector *new_vec() {
    Vector *v = try_memory_allocation(sizeof(Vector));
    v->body = try_memory_allocation(16 * sizeof(void *));
    v->capacity = 16;
    v->len = 0;
    return v;
}

void vec_push(Vector *v, void *elem) {
    if (v->len == v->capacity) {
        // cannot use realloc() here, since the memory has to be zero-cleared in order for this compiler to work
        void *new_p = try_memory_allocation(sizeof(void *) * v->capacity * 2);
        memcpy(new_p, v->body, sizeof(void *) * v->capacity);
        v->body = new_p;
        v->capacity *= 2;
    }
    v->body[v->len++] = elem;
}

void *vec_pop(Vector *v) {
    assert(v->len);
    return v->body[--v->len];
}

void *vec_last(Vector *v) {
    assert(v->len);
    return v->body[v->len - 1];
}

void vec_delete(Vector *v, int index) {
    assert(0 <= index && index < v->len);
    for (int i = index; i < v->len - 1; i++) {
        v->body[i] = v->body[i + 1];
    }
    vec_pop(v);
}

bool vec_contains(Vector *v, void *elem) {
    for (int i = 0; i < v->len; i++)
        if (v->body[i] == elem)
            return true;
    return false;
}

bool vec_union(Vector *v, void *elem) {
    if (vec_contains(v, elem))
        return false;
    vec_push(v, elem);
    return true;
}

void vec_concat(Vector *to, Vector *from) {
    for (int i = 0; i < from->len; i++) {
        vec_push(to, from->body[i]);
    }
}