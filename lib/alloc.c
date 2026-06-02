#include "alloc.h"
#include "mem.h"
#include "klogf.h"
#include "panic.h"

#define HEAP_MAGIC 0xF00DF00D

typedef struct heap_block {
	uint32_t magic;
	size_t size;
	struct heap_block* next;
	bool free;
} heap_block_t;

static heap_block_t* g_heap_start = nullptr;
static size_t g_heap_total_size = 0;

void kalloc_init(void* start, size_t size) {
	if (size < sizeof(heap_block_t)) {
		kpanic("Heap size too small");
	}

	g_heap_start = (heap_block_t*)start;
	g_heap_total_size = size;

	g_heap_start->magic = HEAP_MAGIC;
	g_heap_start->size = size - sizeof(heap_block_t);
	g_heap_start->next = nullptr;
	g_heap_start->free = true;

	klog(LOG_INFO, "Heap initialized at %p, size %u bytes", start, size);
}

void* kmalloc(size_t size) {
	if (size == 0) return nullptr;

	// Align size to 8 bytes
	size = (size + 7) & ~7;

	heap_block_t* current = g_heap_start;
	while (current) {
		if (current->free && current->size >= size) {
			// Can we split this block?
			if (current->size >= size + sizeof(heap_block_t) + 8) {
				heap_block_t* next_block = (heap_block_t*)((uint8_t*)current + sizeof(heap_block_t) + size);
				next_block->magic = HEAP_MAGIC;
				next_block->size = current->size - size - sizeof(heap_block_t);
				next_block->next = current->next;
				next_block->free = true;

				current->size = size;
				current->next = next_block;
			}

			current->free = false;
			return (void*)((uint8_t*)current + sizeof(heap_block_t));
		}
		current = current->next;
	}

	klog(LOG_ERROR, "kmalloc failed to allocate %u bytes", size);
	return NULL;
}

void kfree(void* ptr) {
	if (!ptr) return;

	heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
	if (block->magic != HEAP_MAGIC) {
		kpanic("kfree: invalid magic (double free or heap corruption?)");
	}

	block->free = true;

	// Coalesce adjacent free blocks
	heap_block_t* current = g_heap_start;
	while (current) {
		if (current->free && current->next && current->next->free) {
			current->size += sizeof(heap_block_t) + current->next->size;
			current->next = current->next->next;
			// Don't advance, check if the new next is also free
			continue;
		}
		current = current->next;
	}
}

void* kcalloc(size_t nmemb, size_t size) {
	size_t total = nmemb * size;
	void* ptr = kmalloc(total);
	if (ptr) {
		memset(ptr, 0, total);
	}
	return ptr;
}
