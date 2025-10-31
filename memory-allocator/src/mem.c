#define _DEFAULT_SOURCE

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mem_internals.h"
#include "mem.h"
#include "util.h"

void debug_block(struct block_header *b, const char *fmt, ...);
void debug(const char *fmt, ...);

extern inline block_size size_from_capacity(block_capacity capacity);
extern inline block_capacity capacity_from_size(block_size size);
extern inline bool region_is_invalid(const struct region *r);

static bool block_is_big_enough(size_t query, struct block_header *block) {
    return block->capacity.bytes >= query;
}

static size_t pages_count(size_t mem) {
    return (mem + getpagesize() - 1) / getpagesize();
}

static size_t round_pages(size_t mem) {
    return getpagesize() * pages_count(mem);
}

static void block_init(void *restrict addr, block_size size, void *restrict next) {
    struct block_header *header = addr;
    *header = (struct block_header) {
        .next = next,
        .capacity = capacity_from_size(size),
        .is_free = true
    };
}

static size_t region_actual_size(size_t query) {
    return size_max(round_pages(query), REGION_MIN_SIZE);
}

static void *map_pages(const void *addr, size_t length, int additional_flags) {
    return mmap(
        (void *) addr,
        length,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | additional_flags,
        -1, // fd
        0 // offset
    );
}

/*  аллоцировать регион памяти и инициализировать его блоком */
static struct region alloc_region(const void *addr, size_t query) {
	size_t size_from_query = size_from_capacity((block_capacity) {query}).bytes;
	size_t region_size = region_actual_size(size_from_query);
    void *region_addr = map_pages(addr, region_size, MAP_FIXED_NOREPLACE);

	bool extends = true;

	if (region_addr == MAP_FAILED) {
		extends = false;
		region_addr = map_pages(addr, region_size, 0);
	}

	if (region_addr == MAP_FAILED)
		return REGION_INVALID;

	block_init(region_addr, (block_size) {region_size}, NULL);

    return (struct region) {
        .addr = region_addr,
        .size = region_size,
        .extends = extends
    };
}

void *heap_init(size_t initial) {
    const struct region region = alloc_region(HEAP_START, initial);

	if (region_is_invalid(&region)) return NULL;

    return region.addr;
}

#define BLOCK_MIN_CAPACITY 24

/*  --- Разделение блоков (если найденный свободный блок слишком большой )--- */

static inline size_t min_capacity_or(size_t query) {
	return size_max(BLOCK_MIN_CAPACITY, query);
}

static bool block_splittable(struct block_header *restrict block, size_t query) {
	if (!block || !block->is_free) return false;

    block_capacity min_capacity = (block_capacity) { BLOCK_MIN_CAPACITY };

	size_t required_capacity = min_capacity_or(query) + size_from_capacity(min_capacity).bytes;

    return required_capacity <= block->capacity.bytes;
}

static bool split_if_too_big(struct block_header *block, size_t query) {
    if (block && block_splittable(block, query)) {
        struct block_header *new_block = (void *) (block->contents + min_capacity_or(query));
        block_size new_block_size = (block_size) {block->capacity.bytes - min_capacity_or(query)};

        block_init(new_block, new_block_size, block->next);

        block->next = new_block;
        block->capacity = (block_capacity) { min_capacity_or(query) };

        return true;
    }

    return false;
}

/*  --- Слияние соседних свободных блоков --- */

static void *block_after(const struct block_header *block) {
    return (void *) (block->contents + block->capacity.bytes);
}

static bool blocks_continuous(const struct block_header *first, const struct block_header *second) {
    return first && ((void *)second == block_after(first));
}

static bool mergeable(const struct block_header *restrict first, const struct block_header *restrict second) {
    return first && second && first->is_free && second->is_free && blocks_continuous(first, second);
}

static bool try_merge_with_next(struct block_header *block) {
    if (block && block->next && mergeable(block, block->next)) {
        size_t next_block_size = size_from_capacity(block->next->capacity).bytes;
        block->next = block->next->next;
        block->capacity.bytes += next_block_size;

        return true;
    }

    return false;
}

/*  --- ... ecли размера кучи хватает --- */

enum bsr_type {
	BSR_FOUND_GOOD_BLOCK,
	BSR_REACHED_END_NOT_FOUND,
	BSR_CORRUPTED
};

struct block_search_result {
    enum bsr_type type;
    struct block_header *block;
};

static inline struct block_search_result bsr_create(enum bsr_type type, struct block_header *block) {
	return (struct block_search_result) {
		.type = type,
		.block = block
	};
}

static struct block_search_result
find_good_or_last(struct block_header *restrict block, size_t size) {
	struct block_header *prev = NULL;
    while (block) {
		while (try_merge_with_next(block)) {}

        if (block->is_free && block_is_big_enough(size, block)) {
            return bsr_create(BSR_FOUND_GOOD_BLOCK, block);
        }
		prev = block;
        block = block->next;
    }

    return bsr_create(prev ? BSR_REACHED_END_NOT_FOUND : BSR_CORRUPTED, prev);
}

/*  Попробовать выделить память в куче начиная с блока `block` не пытаясь расширить кучу
 Можно переиспользовать как только кучу расширили. */
static struct block_search_result
try_memalloc_existing(size_t query, struct block_header *block) {
	struct block_search_result result = find_good_or_last(block, query);

	if (result.type == BSR_FOUND_GOOD_BLOCK) {
		split_if_too_big(result.block, query);
		result.block->is_free = false;
	}

	return result;
}

static struct block_header *grow_heap(struct block_header *restrict last, size_t query) {
	void *next_region_addr = block_after(last);

	struct region next_region = alloc_region(next_region_addr, query);
	if (region_is_invalid(&next_region)) {
		return NULL;
	}

	struct block_header *block = next_region.addr;
	last->next = block;

	if (next_region.extends && try_merge_with_next(last)) {
		block = last;
	}


	return block;
}

/*  Реализует основную логику malloc и возвращает заголовок выделенного блока */
static struct block_header *memalloc(size_t query, struct block_header *heap_start) {
	struct block_search_result bsr = try_memalloc_existing(query, heap_start);
	struct block_header *result = NULL;
	switch(bsr.type) {
		case BSR_FOUND_GOOD_BLOCK:
			result = bsr.block;
			break;

		case BSR_CORRUPTED:
			break;

		case BSR_REACHED_END_NOT_FOUND: {
			struct block_header *heap_end = bsr.block;
			result = grow_heap(heap_end, query);
			if (result) {
				split_if_too_big(result, query);
				result->is_free = false;
			}
		}
	}

	return result;
}

void *_malloc(size_t query) {
    struct block_header *const addr = memalloc(query, (struct block_header *) HEAP_START);
    if (addr) return addr->contents;

    return NULL;
}

static struct block_header *block_get_header(void *contents) {
    uint8_t *byte_addr = contents;
    return (struct block_header *) (byte_addr - HEADER_SIZE);
}

void _free(void *mem) {
    if (mem == NULL) return;

    struct block_header *block = block_get_header(mem);
	block->is_free = true;
	try_merge_with_next(block);
}
