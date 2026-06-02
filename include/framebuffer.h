#pragma once
#include <stdint.h>
#include "boot/multiboot2.h"

#define LOGO_WIDTH 64
#define LOGO_HEIGHT 64

#define LOGO_GAP 4

typedef struct {
	uint64_t addr;
	uint32_t width;
	uint32_t height;
	uint32_t pitch;
	uint8_t bpp;
} fb_info_t;

void fb_init(const mb2_tag_framebuffer_t* tag);
void fb_put_pixel(uint32_t x, uint32_t y, uint32_t color);
uint32_t fb_color(uint8_t r, uint8_t g, uint8_t b);
void fb_clear(uint32_t color);
void fb_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);

void fb_draw_logo(uint32_t x, uint32_t y);
void fb_draw_cpu_logos(uint32_t cpu_count);

uint64_t fb_get_addr();
uint32_t fb_get_width();
uint32_t fb_get_height();
uint32_t fb_get_pitch();
uint32_t fb_get_pixel(uint32_t x, uint32_t y);
