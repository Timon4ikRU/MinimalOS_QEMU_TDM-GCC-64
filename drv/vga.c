#include "vga.h"

#define VGA_ADDRESS 0xB8000
#define BUFSIZE (80 * 25)

static uint16_t* const vga_buffer = (uint16_t*)VGA_ADDRESS;
static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t color = 0x0F;

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void vga_set_color(uint8_t new_color) {
    color = new_color;
}

void update_cursor(int x, int y) {
    cursor_x = x;
    cursor_y = y;
    uint16_t pos = y * 80 + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void get_cursor_pos(int *x, int *y) {
    *x = cursor_x;
    *y = cursor_y;
}

void kclear_screen(void) {
    for (int i = 0; i < BUFSIZE; i++) {
        vga_buffer[i] = (uint16_t)color << 8 | ' ';
    }
    update_cursor(0, 0);
}

void kprint_char(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = 79;
        }
        vga_buffer[cursor_y * 80 + cursor_x] = (uint16_t)color << 8 | ' ';
    } else {
        vga_buffer[cursor_y * 80 + cursor_x] = (uint16_t)color << 8 | c;
        cursor_x++;
        if (cursor_x >= 80) {
            cursor_x = 0;
            cursor_y++;
        }
    }

    if (cursor_y >= 25) {
        for (int i = 0; i < 24 * 80; i++) {
            vga_buffer[i] = vga_buffer[i + 80];
        }
        for (int i = 24 * 80; i < 25 * 80; i++) {
            vga_buffer[i] = (uint16_t)color << 8 | ' ';
        }
        cursor_y = 24;
    }

    update_cursor(cursor_x, cursor_y);
}

void kprint(const char* str) {
    while (*str) {
        kprint_char(*str++);
    }
}