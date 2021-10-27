#include "9cc.h"

Vector *new_vec()
{
    Vector *v = calloc(1, sizeof(Vector));
    v->body = calloc(16, sizeof(void *));
    v->capacity = 16;
    v->len = 0;
    return v;
}

void vec_push(Vector *v, void *elem)
{
    if (v->len == v->capacity)
    {
        v->capacity *= 2;
        v->body = realloc(v->body, sizeof(void *) * v->capacity);
    }
    v->body[v->len++] = elem;
}

void vec_pushi(Vector *v, int val)
{
    vec_push(v, &val);
}

void *vec_pop(Vector *v)
{
    assert(v->len);
    return v->body[--v->len];
}

void *vec_last(Vector *v)
{
    assert(v->len);
    return v->body[v->len - 1];
}

bool vec_contains(Vector *v, void *elem)
{
    for (int i = 0; i < v->len; i++)
        if (v->body[i] == elem)
            return true;
    return false;
}

bool vec_union1(Vector *v, void *elem)
{
    if (vec_contains(v, elem))
        return false;
    vec_push(v, elem);
    return true;
}