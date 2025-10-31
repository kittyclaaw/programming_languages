#include "test.h"

#include "mem.h"
#include "mem_internals.h"

#include <stdbool.h>

#define HEAP_SIZE 8192
#define BLOCK_HEADER_OFFSET offsetof(struct block_header, contents)

void heap_destroy(void *heap_start, size_t heap_size) {
	munmap(heap_start, size_from_capacity((block_capacity) { heap_size }).bytes);
}

static bool test_malloc(void) {
	heap_init(HEAP_SIZE);

	fprintf(stderr, "=== Initial heap state ===\n");
	debug_heap(stderr, HEAP_START);

	void *addr = _malloc(123);

	fprintf(stderr, "=== Heap after _malloc ===\n");
	debug_heap(stderr, HEAP_START);

	if (addr == NULL) {
		fprintf(stderr, "=== Allocated address is NULL ===\n");

		return false;
	}

	heap_destroy(HEAP_START, HEAP_SIZE);

	return true;
}

static bool test_free_one_block(void) {
	heap_init(HEAP_SIZE);

	fprintf(stderr, "=== Initial heap state ===\n");

	_malloc(1);
	void *addr = _malloc(1488);
	_malloc(1);

	fprintf(stderr, "=== Heap after _malloc (1, 1488, 1) ===\n");
	debug_heap(stderr, HEAP_START);

	_free(addr);

	fprintf(stderr, "=== Heap after _free ===\n");
	debug_heap(stderr, HEAP_START);

	heap_destroy(HEAP_START, HEAP_SIZE);

	return true;
}

static bool test_free_two_blocks(void) {
	heap_init(HEAP_SIZE);

	fprintf(stderr, "=== Initial heap state ===\n");
	debug_heap(stderr, HEAP_START);

	_malloc(1);
	void *addr1 = _malloc(1488);
	void *addr2 = _malloc(1337);
	_malloc(1);

	fprintf(stderr, "=== Heap after _malloc (1, 1488, 1337, 1) ===\n");
	debug_heap(stderr, HEAP_START);

	_free(addr1);

	fprintf(stderr, "=== Heap after first _free ===\n");
	debug_heap(stderr, HEAP_START);

	_free(addr2);

	fprintf(stderr, "=== Heap after second _free ===\n");
	debug_heap(stderr, HEAP_START);

	heap_destroy(HEAP_START, HEAP_SIZE);

	return true;
}

static bool test_heap_extend(void) {
	heap_init(HEAP_SIZE);

	fprintf(stderr, "=== Initial heap state ===\n");
	debug_heap(stderr, HEAP_START);

	void *addr1 = _malloc(HEAP_SIZE);

	fprintf(stderr, "=== Heap after _malloc ===\n");
	debug_heap(stderr, HEAP_START);

	void *addr2 = _malloc(HEAP_SIZE);

	if (addr2 != addr1 + HEAP_SIZE + BLOCK_HEADER_OFFSET) {
		fprintf(stderr, "=== New block does not extend previous ===\n");

		return false;
	}

	fprintf(stderr, "=== Initial heap state ===\n");
	debug_heap(stderr, HEAP_START);

	heap_destroy(HEAP_START, HEAP_SIZE);
	heap_destroy(addr1 + HEAP_SIZE, HEAP_SIZE);

	return true;
}

static bool test_heap_extend_fail(void) {
	heap_init(HEAP_SIZE);

	fprintf(stderr, "=== Initial heap state ===\n");
	debug_heap(stderr, HEAP_START);

	void *addr1 = _malloc(HEAP_SIZE);

	fprintf(stderr, "=== Heap after first _malloc ===\n");
	debug_heap(stderr, HEAP_START);

	_malloc(1);

	void *addr2 = _malloc(HEAP_SIZE);

	fprintf(stderr, "=== Heap after second _malloc ===\n");
	debug_heap(stderr, HEAP_START);

	if (addr2 == addr1 + HEAP_SIZE + BLOCK_HEADER_OFFSET) {
		fprintf(stderr, "=== New block extends previous ===\n");

		return false;
	}

	heap_destroy(HEAP_START, HEAP_SIZE);
	heap_destroy(addr1 + HEAP_SIZE, 1);
	heap_destroy(addr2 - BLOCK_HEADER_OFFSET, HEAP_SIZE);

	return true;
}

typedef bool (*test_f) (void);

static void print_test(test_f test, size_t order, size_t *count) {
	fprintf(stderr, "=== TEST #%zu ===\n", order);

	if (test()) {
		fprintf(stderr, "\n=== TEST #%zu PASSED ===\n\n", order);
	} else {
		++*count;
		fprintf(stderr, "\n=== TEST #%zu FAILED ===\n\n", order);
	}
}

int run_heap_tests(void) {
	size_t count = 0;

	print_test(test_malloc, 1, &count);
	print_test(test_free_one_block, 2, &count);
	print_test(test_free_two_blocks, 3, &count);
	print_test(test_heap_extend, 4, &count);
	print_test(test_heap_extend_fail, 5, &count);

	if (count) {
		fprintf(stderr, "=== %zu/5 TESTS FAILED ===\n", count);

		return 1;
	} else {
		fprintf(stderr, "=== ALL TESTS PASSED ===\n");

		return 0;
	}
}
