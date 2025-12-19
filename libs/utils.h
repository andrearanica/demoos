#ifndef _UTILS_H
#define _UTILS_H

#include <stddef.h>

extern int get_el(void);
extern void delay(unsigned long);
extern size_t strlen(const char*);
extern char* strcat(char*, const char*);
extern char* strcpy(char*, const char*);
extern void strsplit(const char*, char, char*, char*);

#endif
