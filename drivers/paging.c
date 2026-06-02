#include "paging.h"

#include "klogf.h"
#include "mem.h"
#include "panic.h"

static page_table_t* g_pml4 = nullptr;
static uint8_t page_table_pool[4096 * 32] ALIGNED(4096);
static size_t pool_offset = 0;

static page_table_t* alloc_page_table() {
	if (pool_offset + sizeof(page_table_t) > sizeof(page_table_pool))
		kpanic("Page table pool exhausted");

	auto const table = (page_table_t*)(page_table_pool + pool_offset);
	pool_offset += sizeof(page_table_t);
	memset(table, 0, sizeof(page_table_t));
	return table;
}

void map_page_4kb(page_table_t* pml4, const virt_addr_t virt, const phys_addr_t phys, const uint64_t flags) {
	const uint64_t pml4_idx = (virt >> 39) & 0x1FF;
	const uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
	const uint64_t pd_idx = (virt >> 21) & 0x1FF;
	const uint64_t pt_idx = (virt >> 12) & 0x1FF;

	if (!(pml4->entries[pml4_idx] & PAGE_PRESENT)) {
		auto const pdpt = alloc_page_table();
		pml4->entries[pml4_idx] = (uint64_t)pdpt | PAGE_PRESENT | PAGE_WRITABLE;
	}
	auto const pdpt = (page_table_t*)(pml4->entries[pml4_idx] & ~0xFFF);

	if (!(pdpt->entries[pdpt_idx] & PAGE_PRESENT)) {
		auto const pd = alloc_page_table();
		pdpt->entries[pdpt_idx] = (uint64_t)pd | PAGE_PRESENT | PAGE_WRITABLE;
	}
	auto const pd = (page_table_t*)(pdpt->entries[pdpt_idx] & ~0xFFF);

	if (!(pd->entries[pd_idx] & PAGE_PRESENT)) {
		auto const pt = alloc_page_table();
		pd->entries[pd_idx] = (uint64_t)pt | PAGE_PRESENT | PAGE_WRITABLE;
	}
	auto const pt = (page_table_t*)(pd->entries[pd_idx] & ~0xFFF);

	pt->entries[pt_idx] = (phys & ~0xFFFULL) | flags | PAGE_PRESENT;
}

void map_page_2mb(page_table_t* pml4, const virt_addr_t virt, const phys_addr_t phys, const uint64_t flags) {
	const uint64_t pml4_idx = (virt >> 39) & 0x1FF;
	const uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
	const uint64_t pd_idx = (virt >> 21) & 0x1FF;

	// PML4 -> PDPT
	if (!(pml4->entries[pml4_idx] & PAGE_PRESENT)) {
		auto const pdpt = alloc_page_table();
		pml4->entries[pml4_idx] = (uint64_t)pdpt | PAGE_PRESENT | PAGE_WRITABLE;
	}
	auto const pdpt = (page_table_t*)(pml4->entries[pml4_idx] & ~0xFFF);

	// PDPT -> PD
	if (!(pdpt->entries[pdpt_idx] & PAGE_PRESENT)) {
		auto const pd = alloc_page_table();
		pdpt->entries[pdpt_idx] = (uint64_t)pd | PAGE_PRESENT | PAGE_WRITABLE;
	}
	auto const pd = (page_table_t*)(pdpt->entries[pdpt_idx] & ~0xFFF);

	// PD -> 2MiB page
	pd->entries[pd_idx] = (phys & ~0x1FFFFFULL) | flags | PAGE_PRESENT | PAGE_HUGE;
}

void map_region_4kb(page_table_t* pml4, const virt_addr_t virt, const phys_addr_t phys, const uint64_t size, const uint64_t flags) {
	for (uint64_t offset = 0; offset < size; offset += 0x1000) {
		map_page_4kb(pml4, virt + offset, phys + offset, flags);
	}
}

void map_region_2mb(page_table_t* pml4, const virt_addr_t virt, const phys_addr_t phys, const uint64_t size, const uint64_t flags) {
	for (uint64_t offset = 0; offset < size; offset += 0x200000) {
		map_page_2mb(pml4, virt + offset, phys + offset, flags);
	}
}

void paging_switch(page_table_t* pml4) {
	__asm__ volatile("mov %0, %%cr3" : : "r"((uint64_t)pml4) : "memory");
}

void paging_map_mmio(const virt_addr_t addr, const uint64_t size) {
	uint64_t base = addr & ~0xFFFULL;
	uint64_t end  = (addr + size + 0xFFF) & ~0xFFFULL;
	map_region_4kb(g_pml4, base, base, end - base, PAGE_WRITABLE);
}

void paging_map_mmio_region(phys_addr_t phys, uint64_t size) {
	uint64_t base = phys & ~0xFFFULL;
	uint64_t end  = (phys + size + 0xFFF) & ~0xFFFULL;

	map_region_4kb(g_pml4, base, base, end - base, PAGE_WRITABLE | PAGE_CACHE_DISABLE);
}

extern const char ld_kernel_start[];
extern const char ld_kernel_end[];

page_table_t* paging_init(mb2_info_t* info) {
	if (g_pml4)
		kpanic("Pml4 already initialized! Double INIT error");

	g_pml4 = alloc_page_table();

	uint64_t kernel_start = (uint64_t)ld_kernel_start;
	uint64_t kernel_end   = (uint64_t)ld_kernel_end;
	uint64_t kernel_size  = kernel_end - kernel_start;

	// Round up to nearest 2MiB
	kernel_size = (kernel_size + 0x1FFFFF) & ~0x1FFFFF;

	uint64_t map_size = kernel_size + sizeof(page_table_pool);
	map_size = (map_size + 0x1FFFFF) & ~0x1FFFFF;

	// Map low memory (stack, VGA, BIOS, etc.)
	map_region_2mb(g_pml4, 0x0, 0x0, 0x200000, PAGE_WRITABLE);

	// Map the kernel
	map_region_2mb(g_pml4, kernel_start, kernel_start, map_size, PAGE_WRITABLE);

	// Walk memory map and map all usable regions
	auto tag = (mb2_tag_t*)info->tags;
	while (tag->type != MB2_TAG_END) {
		if (tag->type == MB2_TAG_MMAP) {
			auto mmap = (mb2_tag_mmap_t*)tag;
			auto entry = mmap->entries;
			while ((uint8_t*)entry < (uint8_t*)mmap + mmap->tag.size) {
				if (entry->type == 1) {  // usable RAM
					uint64_t base = entry->base_addr & ~0x1FFFFFULL;  // align down to 2MiB
					uint64_t end  = (entry->base_addr + entry->length + 0x1FFFFF) & ~0x1FFFFFULL;
					klog(LOG_INFO, "Mapping region: 0x%lx - 0x%lx", base, end);
					map_region_2mb(g_pml4, base, base, end - base, PAGE_WRITABLE);
				}
				entry = (mb2_mmap_entry_t*)((uint8_t*)entry + mmap->entry_size);
			}
		}
		tag = (mb2_tag_t*)((uint8_t*)tag + ((tag->size + 7) & ~7));
	}


	paging_switch(g_pml4);
	return g_pml4;
}

