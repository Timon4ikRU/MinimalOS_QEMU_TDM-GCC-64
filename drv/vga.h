#ifndef VGA_H
#define VGA_H

#include <stdint.h>

void kclear_screen(void);
void kprint(const char* str);
void kprint_char(char c);
void update_cursor(int x, int y);
void get_cursor_pos(int *x, int *y);
void vga_set_color(uint8_t new_color);

#endif