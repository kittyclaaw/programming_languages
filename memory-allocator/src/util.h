#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

inline size_t size_max(size_t x, size_t y) {
    return x > y ? x : y;
}

inline size_t size_min(size_t x, size_t y) {
    return x < y ? x : y;
}

_Noreturn void err( const char* msg, ... );


#endif
