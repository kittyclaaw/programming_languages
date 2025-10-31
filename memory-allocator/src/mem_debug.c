#include <stdarg.h>
#include <stdio.h>

#include "mem.h"
#include "mem_internals.h"
#include "util.h"

extern inline size_t size_min(size_t x, size_t y);

void debug_struct_info(FILE *f, const void *addr) {
  const struct block_header *header = addr;
  size_t capacity = header->capacity.bytes;
  char *free_status = header->is_free ? "free" : "taken";

  fprintf(f, "%10p %10zu %8s   ", addr, capacity, free_status);

  size_t to_print = size_min(DEBUG_FIRST_BYTES, capacity);
  for (size_t i = 0; i < to_print; ++i) {
    fprintf(f, "%hhX", header->contents[i]);
  }

  fprintf(f, "\n");
}

void debug_heap(FILE *f, const void *addr) {
  fprintf(f, " --- Heap ---\n");
  fprintf(f, "%10s %10s %8s %10s\n", "start", "capacity", "status", "contents");
  for (
      const struct block_header *header = addr;
      header; header = header->next
  ) {
    debug_struct_info(f, header);
  }
}

void debug_block(struct block_header *b, const char *fmt, ...) {
#ifdef DEBUG
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  debug_struct_info(stderr, b);
  va_end(args);
#else
  (void)b;
  (void)fmt;
#endif
}

void debug(const char *fmt, ...) {
#ifdef DEBUG
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
#else
  (void)fmt;
#endif
}
