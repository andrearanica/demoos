#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>

extern size_t strlen(const char *);
extern char *strcat(char *, const char *);
extern char *strcpy(char *, const char *);
extern void strsplit(const char *, char, char *, char *);
extern int strcmp(const char*, const char*, int);

#endif