#ifndef MEM_H
#define MEM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <sys/mman.h>

#define HEAP_START ((void*)0x04040000)

void *_malloc(size_t query);
void _free(void *mem);
void *heap_init(size_t initial_size);

#define DEBUG_FIRST_BYTES 4

void debug_struct_info(FILE *f, const void *addr);
void debug_heap(FILE *f, const void *addr);

#endif
