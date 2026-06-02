#pragma once
#include <stdint.h>

#include "addr.h"
#include "macros.h"
#include "boot/multiboot2.h"

typedef uint64_t page_entry_t;

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_WRITABLE (1ULL << 1)
#define PAGE_USER (1ULL << 2)
#define PAGE_WRITE_THROUGH (1ULL << 3)
#define PAGE_CACHE_DISABLE (1ULL << 4)
#define PAGE_ACCESSED (1ULL << 5)
#define PAGE_DIRTY (1ULL << 6)
#define PAGE_HUGE (1ULL << 7)
#define PAGE_GLOBAL (1ULL << 8)
#define PAGE_NO_EXECUTE (1ULL << 63)

typedef struct {
	page_entry_t entries[512];
} ALIGNED(4096) page_table_t;

void map_page_4kb(page_table_t* pml4, virt_addr_t virt, phys_addr_t phys, uint64_t flags);
void map_page_2mb(page_table_t* pml4, virt_addr_t virt, phys_addr_t phys, uint64_t flags);
void map_region_4kb(page_table_t* pml4, virt_addr_t virt, phys_addr_t phys, uint64_t size, uint64_t flags);
void map_region_2mb(page_table_t* pml4, virt_addr_t virt, phys_addr_t phys, uint64_t size, uint64_t flags);
void paging_map_mmio(virt_addr_t addr, uint64_t size);
void paging_map_mmio_region(phys_addr_t phys, uint64_t size);
void paging_switch(page_table_t* pml4);
page_table_t* paging_init(mb2_info_t* info);