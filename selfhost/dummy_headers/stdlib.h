#ifndef STDLIB_H
#define STDLIB_H

#include <stdio.h>

#define NULL (void *)0

void *malloc(size_t size);
void *calloc(size_t n, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

#endif