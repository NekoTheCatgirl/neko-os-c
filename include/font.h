#pragma once
#include <stdint.h>

#include "macros.h"

#define PSF1_MAGIC 0x0436
#define PSF2_MAGIC 0x864AB572

typedef struct PACKED {
	uint16_t magic;
	uint8_t mode;
	uint8_t charsize;
} psf1_header_t;

typedef struct PACKED {
	uint32_t magic;        // 0x864AB572
	uint32_t version;      // 0
	uint32_t header_size;  // 32
	uint32_t flags;
	uint32_t glyph_count;
	uint32_t bytes_per_glyph;
	uint32_t height;
	uint32_t width;
} psf2_header_t;

void font_init();
uint32_t font_get_width();
uint32_t font_get_height();
void font_draw_char(char c, uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);