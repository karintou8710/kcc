#ifndef STRING_H
#define STRING_H

#include <stdio.h>

int memcmp(void *buf1, void *buf2, size_t n);
int strcmp(char *s1, char *s2);
int strncmp(char *s1, char *s2, size_t n);
void *memcpy(void *buf1, void *buf2, size_t n);
size_t strlen(char *s);
char *strerror(int errnum);
char *strstr(char *s1, char *s2);
char *strchr(char *s, int c);
long strtol(char *s, char **endptr, int base);
void *memset(void *buf, int ch, size_t n);

#endif