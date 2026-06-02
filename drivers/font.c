#include "font.h"

#include <framebuffer.h>
#include <klogf.h>
#include <panic.h>
#include <stddef.h>

struct font_asset {
	const uint8_t* data;
	size_t size;
};

extern const uint8_t _binary_ter_v16n_psf_start[];
extern const uint8_t _binary_ter_v16n_psf_end[];

static inline struct font_asset font_get() {
	return (struct font_asset){
		.data = _binary_ter_v16n_psf_start,
		.size = (size_t)(_binary_ter_v16n_psf_end - _binary_ter_v16n_psf_start)
	};
}

static psf1_header_t* font_header_v1;
static psf2_header_t* font_header_v2;
static uint8_t* font_glyphs;
static uint32_t font_width, font_height, font_glyph_size;

void font_init() {
	const uint8_t* data = _binary_ter_v16n_psf_start;

	if (*(uint16_t*)data == PSF1_MAGIC) {
		font_header_v1 = (psf1_header_t*)data;
		font_glyphs = (uint8_t*)data + sizeof(psf1_header_t);
		font_width = 8;
		font_height = font_header_v1->charsize;
		font_glyph_size = font_header_v1->charsize;
		klog(LOG_INFO, "PSF1 font detected");
	} else if (*(uint32_t*)data == PSF2_MAGIC) {
		font_header_v2 = (psf2_header_t*)data;
		font_glyphs = (uint8_t*)data + font_header_v2->header_size;
		font_width = font_header_v2->width;
		font_height = font_header_v2->height;
		font_glyph_size = font_header_v2->bytes_per_glyph;
		klog(LOG_INFO, "PSF2 font detected");
	} else {
		kpanic("Invalid PSF font (not embedded correctly) 0x%x", *(uint32_t*)data);
	}
}

uint32_t font_get_width() {
	return font_width;
}

uint32_t font_get_height() {
	return font_height;
}

void font_draw_char(const char c, const uint32_t x, const uint32_t y, const uint32_t fg, const uint32_t bg) {
	if (!font_glyphs) return;

	const uint8_t* glyph = font_glyphs + (uint8_t)c * font_glyph_size;
	const uint32_t bytes_per_row = (font_width + 7) / 8;

	for (uint32_t row = 0; row < font_height; row++) {
		for (uint32_t col = 0; col < font_width; col++) {
			const uint8_t byte = glyph[row * bytes_per_row + col / 8];
			const uint32_t color = (byte >> (7 - (col % 8))) & 1 ? fg : bg;
			fb_put_pixel(x + col, y + row, color);
		}
	}
}
