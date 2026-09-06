#include "keyboard.h"
#include "vga.h"

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static const char scancode_ascii_normal[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,  '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
  '*',   0, ' '
};

static const char scancode_ascii_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,  '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
  '*',   0, ' '
};

static int caps_lock = 0;
static int shift_pressed = 0;
static int ctrl_pressed = 0;
static int alt_pressed = 0;
static int is_extended = 0;

#define KBD_BUFFER_SIZE 64
static char kbd_buffer[KBD_BUFFER_SIZE];
static volatile int buffer_head = 0;
static volatile int buffer_tail = 0;

static void buffer_put(char c) {
    int next = (buffer_head + 1) % KBD_BUFFER_SIZE;
    if (next != buffer_tail) {
        kbd_buffer[buffer_head] = c;
        buffer_head = next;
    }
}

void keyboard_handler(void) {
    uint8_t scancode = inb(0x60);
    outb(0x20, 0x20);

    if (scancode == 0xE0) {
        is_extended = 1;
        return;
    }

    if (is_extended) {
        is_extended = 0;
        if (scancode == 0x48) buffer_put((char)KEY_UP);
        else if (scancode == 0x50) buffer_put((char)KEY_DOWN);
        else if (scancode == 0x4B) buffer_put((char)KEY_LEFT);
        else if (scancode == 0x4D) buffer_put((char)KEY_RIGHT);
        return;
    }

    if (scancode == 0x38) { alt_pressed = 1; return; }
    if (scancode == 0xB8) { alt_pressed = 0; return; }
    if (scancode == 0x1D) { ctrl_pressed = 1; return; }
    if (scancode == 0x9D) { ctrl_pressed = 0; return; }
    if (scancode == 0x2A || scancode == 0x36) { shift_pressed = 1; return; }
    if (scancode == 0xAA || scancode == 0xB6) { shift_pressed = 0; return; }
    if (scancode == 0x3A) { caps_lock = !caps_lock; return; }

    if (scancode & 0x80) return;

    char c = shift_pressed ? scancode_ascii_shift[scancode] : scancode_ascii_normal[scancode];

    // Checking for Alt + Shift + J (scancode 0x24 - J key)
    if (alt_pressed && shift_pressed && (c == 'J' || c == 'j')) {
        buffer_put(ALT_SHIFT_J);
        return;
    }

    if (ctrl_pressed) {
        if (c == 'c' || c == 'C') buffer_put(CTRL_C);
        else if (c == 'z' || c == 'Z') buffer_put(CTRL_Z);
        else if (c == 's' || c == 'S') buffer_put(CTRL_S);
        else if (c == 'q' || c == 'Q') buffer_put(CTRL_Q);
        return;
    }

    if (caps_lock) {
        if (c >= 'a' && c <= 'z') c -= 32;
        else if (c >= 'A' && c <= 'Z') c += 32;
    }

    if (c != 0) {
        buffer_put(c);
    }
}

char keyboard_read_char(void) {
    while (buffer_head == buffer_tail);
    char c = kbd_buffer[buffer_tail];
    buffer_tail = (buffer_tail + 1) % KBD_BUFFER_SIZE;
    return c;
}