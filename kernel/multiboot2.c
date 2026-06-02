#include "boot/multiboot2.h"

#include "framebuffer.h"
#include "klogf.h"


void mb2_parse(mb2_info_t* info) {
	auto tag = (mb2_tag_t*)info->tags;

	while (tag->type != MB2_TAG_END) {
		switch (tag->type) {
			case MB2_TAG_CMDLINE:
				klog(LOG_INFO, "Cmdline: %s", ((mb2_tag_string_t*)tag)->string);
				break;

			case MB2_TAG_BOOTLOADER:
				klog(LOG_INFO, "Bootloader: %s", ((mb2_tag_string_t*)tag)->string);
				break;

			case MB2_TAG_MMAP: {
				mb2_tag_mmap_t* mmap  = (mb2_tag_mmap_t*)tag;
				int             count = (mmap->tag.size - sizeof(mb2_tag_mmap_t)) / mmap->entry_size;
				klog(LOG_INFO, "Memory map: %d entries", count);
				for (int i = 0; i < count; i++) {
					mb2_mmap_entry_t* e = &mmap->entries[i];
					klog(LOG_INFO, "  0x%x - 0x%x type=%d",
						 e->base_addr, e->base_addr + e->length, e->type);
				}
				break;
			}

			case MB2_TAG_ACPI_V1: {
				rsdp_v1_t* rsdp = (rsdp_v1_t*)((mb2_tag_acpi_t*)tag)->rsdp;
				klog(LOG_INFO, "ACPI 1.0 RSDT at 0x%x", rsdp->rsdt_address);
				break;
			}

			case MB2_TAG_ACPI_V2: {
				rsdp_v2_t* rsdp = (rsdp_v2_t*)((mb2_tag_acpi_t*)tag)->rsdp;
				klog(LOG_INFO, "ACPI 2.0 XSDT at 0x%x", rsdp->xsdt_address);
				break;
			}

			case MB2_TAG_FRAMEBUFFER: {
				mb2_tag_framebuffer_t* fb = (mb2_tag_framebuffer_t*)tag;
				klog(LOG_INFO, "Framebuffer: %dx%d @ %d bpp at 0x%x",
					fb->framebuffer_width,
					fb->framebuffer_height,
					fb->framebuffer_bpp,
					fb->framebuffer_addr);
				fb_init(fb);
				break;
			}
		}

		// Tags are 8-byte aligned
		tag = (mb2_tag_t*)((uint8_t*)tag + ((tag->size + 7) & ~7));
	}
}

rsdp_v1_t* get_acpi_rsdp(mb2_info_t* info) {
	auto tag = (mb2_tag_t*)info->tags;

	rsdp_v1_t* result = nullptr;

	while (tag->type != MB2_TAG_END) {
		if (tag->type == MB2_TAG_ACPI_V2) {
			result = (rsdp_v1_t*)((mb2_tag_acpi_t*)tag)->rsdp;
			break; // V2 is always preferred, stop immediately
		}
		if (tag->type == MB2_TAG_ACPI_V1) {
			result = (rsdp_v1_t*)((mb2_tag_acpi_t*)tag)->rsdp;
			uint8_t* raw_rsdp = (uint8_t*)((mb2_tag_acpi_t*)tag)->rsdp;
			klog(LOG_INFO, "RSDP bytes: %x %x %x %x %x %x %x %x",
				(uint64_t)raw_rsdp[0], (uint64_t)raw_rsdp[1],
				(uint64_t)raw_rsdp[2], (uint64_t)raw_rsdp[3],
				(uint64_t)raw_rsdp[4], (uint64_t)raw_rsdp[5],
				(uint64_t)raw_rsdp[6], (uint64_t)raw_rsdp[7]);
		}
		tag = (mb2_tag_t*)((uint8_t*)tag + ((tag->size + 7) & ~7));
	}

	return result;
}
