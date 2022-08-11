#ifndef STDLIB_H
#define STDLIB_H

#include <stdio.h>

#define NULL (void *)0

void *malloc(size_t size);
void *calloc(size_t n, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
void srand(unsigned int seed);
int rand(void);
int system(const char *command);
char *realpath(const char *p1, char *p2);
long int random(void);
void srandom(unsigned int);

#endif