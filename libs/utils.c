#include "utils.h"

// Returns the length of the given string
size_t strlen(const char* str) {
    size_t len = 0;

    while (str[len] != '\0') {
        len++;
    }

    return len;
}

// Concatenates the first string with the second one, storing the result
// in the first one
char* strcat(char* dest, const char* src) {
    char* pointer = dest;

    while (*pointer != '\0') {
        pointer++;
    }

    while (*src != '\0') {
        *pointer = *src;
        pointer++;
        src++;
    }

    *pointer = '\0';

    return dest;
}

// Copies in the first string the content of the second one
char* strcpy(char* dest, const char* src) {
    char* dest_start = dest;
    char* pointer = src;
    while (*pointer != '\0') {
        *dest = *pointer;
        pointer++;
        dest++;
    }
    *dest = '\0';

    return dest_start;
}
