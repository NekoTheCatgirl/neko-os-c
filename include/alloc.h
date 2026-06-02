#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Initialize the kernel heap allocator.
 * 
 * @param start Start address of the heap.
 * @param size Size of the heap in bytes.
 */
void kalloc_init(void* start, size_t size);

/**
 * @brief Allocate a block of memory from the kernel heap.
 * 
 * @param size Number of bytes to allocate.
 * @return Pointer to the allocated memory, or NULL if allocation failed.
 */
void* kmalloc(size_t size);

/**
 * @brief Free a block of memory previously allocated with kmalloc.
 * 
 * @param ptr Pointer to the memory block to free.
 */
void kfree(void* ptr);

/**
 * @brief Allocate a block of memory and initialize it to zero.
 * 
 * @param nmemb Number of elements.
 * @param size Size of each element.
 * @return Pointer to the allocated memory, or NULL if allocation failed.
 */
void* kcalloc(size_t nmemb, size_t size);
