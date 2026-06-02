#include "framebuffer.h"
#include "paging.h"

static fb_info_t fb;

void fb_init(const mb2_tag_framebuffer_t* tag) {
	fb.addr   = tag->framebuffer_addr;
	fb.width  = tag->framebuffer_width;
	fb.height = tag->framebuffer_height;
	fb.pitch  = tag->framebuffer_pitch;
	fb.bpp    = tag->framebuffer_bpp;

	const uint64_t size = fb.pitch * fb.height;

	// Align to page boundary
	const uint64_t base = fb.addr & ~0xFFF;
	const uint64_t end  = (fb.addr + size + 0xFFF) & ~0xFFF;

	paging_map_mmio_region(base, end - base);
}

void fb_put_pixel(const uint32_t x, const uint32_t y, const uint32_t color) {
	if (x >= fb.width || y >= fb.height) return;

	uint8_t* pixel = (uint8_t*)fb.addr
		+ y * fb.pitch
		+ x * (fb.bpp / 8);

	pixel[0] = color & 0xFF;        // Blue
	pixel[1] = (color >> 8) & 0xFF; // Green
	pixel[2] = (color >> 16) & 0xFF;// Red
}

uint32_t fb_color(const uint8_t r, const uint8_t g, const uint8_t b) {
	return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

void fb_clear(const uint32_t color) {
	for (uint32_t y = 0; y < fb.height; y++)
		for (uint32_t x = 0; x < fb.width; x++)
			fb_put_pixel(x, y, color);
}

void fb_fill_rect(const uint32_t x, const uint32_t y, const uint32_t w, const uint32_t h, const uint32_t color) {
	for (uint32_t row = y; row < y + h; row++)
		for (uint32_t col = x; col < x + w; col++)
			fb_put_pixel(col, row, color);
}

extern uint8_t _binary_logo_raw_start[];

void fb_draw_logo(uint32_t x, uint32_t y) {
	uint8_t* px = _binary_logo_raw_start;
	for (uint32_t row = 0; row < LOGO_WIDTH; row++) {
		for (uint32_t col = 0; col < LOGO_HEIGHT; col++) {
			uint8_t r = *px++;
			uint8_t g = *px++;
			uint8_t b = *px++;
			uint8_t a = *px++;

			if (a == 0) continue;  // fully transparent, skip

			if (a == 255) {
				fb_put_pixel(x + col, y + row, fb_color(r, g, b));
			} else {
				// Alpha blend against current background
				uint32_t bg = fb_get_pixel(x + col, y + row);
				uint8_t bg_r = (bg >> 16) & 0xFF;
				uint8_t bg_g = (bg >> 8)  & 0xFF;
				uint8_t bg_b =  bg        & 0xFF;

				uint8_t out_r = (r * a + bg_r * (255 - a)) / 255;
				uint8_t out_g = (g * a + bg_g * (255 - a)) / 255;
				uint8_t out_b = (b * a + bg_b * (255 - a)) / 255;

				fb_put_pixel(x + col, y + row, fb_color(out_r, out_g, out_b));
			}
		}
	}
}

void fb_draw_cpu_logos(uint32_t cpu_count) {
	uint32_t total_width = cpu_count * (LOGO_WIDTH + LOGO_GAP) - LOGO_GAP;
	uint32_t x = (fb_get_width() - total_width) / 2;  // centered
	uint32_t y = fb_get_height() - LOGO_HEIGHT - 10;   // near bottom

	for (uint32_t i = 0; i < cpu_count; i++) {
		fb_draw_logo(x + i * (LOGO_WIDTH + LOGO_GAP), y);
	}
}

uint64_t fb_get_addr() { return fb.addr; }
uint32_t fb_get_width() { return fb.width; }
uint32_t fb_get_height() { return fb.height; }
uint32_t fb_get_pitch() { return fb.pitch; }
uint32_t fb_get_pixel(const uint32_t x, const uint32_t y) {
	const uint8_t* pixel = (uint8_t*)(fb.addr + y * fb.pitch + x * (fb.bpp / 8));
	return ((uint32_t)pixel[0] << 16) | ((uint32_t)pixel[1] << 8) | pixel[2];
}
