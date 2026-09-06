#include "drv/vga.h"
#include "drv/keyboard.h"
#include "drv/ata.h"
#include "drv/idt.h"
#include "drv/speaker.h"
#include "drv/fat.h"

// Sorry for no comments here. It's not an Assembly tho, it's still readable.

#define CMD_BUFFER_SIZE 128
#define FILE_SIZE 512

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) { s1++; s2++; n--; }
    if (n == 0) return 0;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

static uint8_t hex_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

void play_melody(void) {
    uint32_t notes[] = {261, 293, 329, 349, 392, 440, 493, 523};
    for (int i = 0; i < 8; i++) {
        sound_beep(notes[i], 500);
        sound_beep(0, 100);
    }
}

void init_registry(void) {
    // Some ugly sh#t going on here. Beware.
    sound_set_enabled(1);

    fat_cd("sys");
    char reg_buf[128];
    for (int i = 0; i < 128; i++) reg_buf[i] = 0;

    int len = fat_read_file("sysmain.registry", reg_buf, 128);

    if (len <= 0) {
        const char* default_reg = "COLOR=0x0F\nSOUND=1\n";
        fat_write_file("sysmain.registry", default_reg, 20);
        vga_set_color(0x0F);
    } else {
        for (int i = 0; i < len - 8; i++) {
            if (strncmp(&reg_buf[i], "COLOR=0x", 8) == 0) {
                uint8_t high = hex_to_int(reg_buf[i + 8]);
                uint8_t low = hex_to_int(reg_buf[i + 9]);
                uint8_t color_val = (high << 4) | low;
                vga_set_color(color_val);
                break;
            }
        }
        for (int i = 0; i < len - 6; i++) {
            if (strncmp(&reg_buf[i], "SOUND=", 6) == 0) {
                if (reg_buf[i + 6] == '0') sound_set_enabled(0);
                else if (reg_buf[i + 6] == '1') sound_set_enabled(1);
                break;
            }
        }
    }
    fat_cd("..");
}

void render_nano(const char* filename, const char* buffer, uint32_t file_len, int is_replace_mode, uint32_t cursor_idx) {
    kclear_screen();
    kprint("=== NANO Editor: ");
    kprint(filename);
    kprint(" ===\n");
    kprint("[Alt+Shift+J: INS/RPL | Ctrl+S: Save | Ctrl+Q: Exit]\n");
    kprint(is_replace_mode ? "MODE: [REPLACE]\n\n" : "MODE: [INSERT]\n\n");

    const int start_row = 4;
    int cur_x = 0;
    int cur_y = start_row;
    int target_x = 0;
    int target_y = start_row;

    for (uint32_t i = 0; i < file_len; i++) {
        if (i == cursor_idx) {
            target_x = cur_x;
            target_y = cur_y;
        }

        char c = buffer[i];
        kprint_char(c);

        if (c == '\n') {
            cur_x = 0;
            cur_y++;
        } else {
            cur_x++;
            if (cur_x >= 80) {
                cur_x = 0;
                cur_y++;
            }
        }
    }

    if (cursor_idx >= file_len) {
        target_x = cur_x;
        target_y = cur_y;
    }

    update_cursor(target_x, target_y);
}

static uint32_t move_cursor_up(const char* buffer, uint32_t cursor_idx) {
    if (cursor_idx == 0) return 0;

    uint32_t cur_line_start = cursor_idx;
    while (cur_line_start > 0 && buffer[cur_line_start - 1] != '\n') {
        cur_line_start--;
    }

    if (cur_line_start == 0) return 0;

    uint32_t col = cursor_idx - cur_line_start;

    uint32_t prev_line_start = cur_line_start - 1;
    while (prev_line_start > 0 && buffer[prev_line_start - 1] != '\n') {
        prev_line_start--;
    }

    uint32_t prev_line_len = (cur_line_start - 1) - prev_line_start;

    if (col > prev_line_len) col = prev_line_len;
    return prev_line_start + col;
}

static uint32_t move_cursor_down(const char* buffer, uint32_t file_len, uint32_t cursor_idx) {
    uint32_t cur_line_start = cursor_idx;
    while (cur_line_start > 0 && buffer[cur_line_start - 1] != '\n') {
        cur_line_start--;
    }

    uint32_t col = cursor_idx - cur_line_start;

    uint32_t next_line_start = cursor_idx;
    while (next_line_start < file_len && buffer[next_line_start] != '\n') {
        next_line_start++;
    }

    if (next_line_start >= file_len) return cursor_idx;
    next_line_start++;

    uint32_t next_line_end = next_line_start;
    while (next_line_end < file_len && buffer[next_line_end] != '\n') {
        next_line_end++;
    }

    uint32_t next_line_len = next_line_end - next_line_start;
    if (col > next_line_len) col = next_line_len;

    return next_line_start + col;
}

void nano_editor(const char* filename) {
    int is_replace_mode = 0;
    char buffer[FILE_SIZE];
    for (int i = 0; i < FILE_SIZE; i++) buffer[i] = 0;

    uint32_t file_len = fat_read_file(filename, buffer, FILE_SIZE);
    uint32_t cursor_idx = 0;

    render_nano(filename, buffer, file_len, is_replace_mode, cursor_idx);

    while (1) {
        char c = keyboard_read_char();

        if ((uint8_t)c == ALT_SHIFT_J) {
            is_replace_mode = !is_replace_mode;
            render_nano(filename, buffer, file_len, is_replace_mode, cursor_idx);
            continue;
        }

        if ((uint8_t)c == KEY_LEFT) {
            if (cursor_idx > 0) {
                cursor_idx--;
                render_nano(filename, buffer, file_len, is_replace_mode, cursor_idx);
            }
            continue;
        }

        if ((uint8_t)c == KEY_RIGHT) {
            if (cursor_idx < file_len) {
                cursor_idx++;
                render_nano(filename, buffer, file_len, is_replace_mode, cursor_idx);
            }
            continue;
        }

        if ((uint8_t)c == KEY_UP) {
            cursor_idx = move_cursor_up(buffer, cursor_idx);
            render_nano(filename, buffer, file_len, is_replace_mode, cursor_idx);
            continue;
        }

        if ((uint8_t)c == KEY_DOWN) {
            cursor_idx = move_cursor_down(buffer, file_len, cursor_idx);
            render_nano(filename, buffer, file_len, is_replace_mode, cursor_idx);
            continue;
        }

        if (c == CTRL_C || c == CTRL_Q) {
            kprint("\n[Exited without saving]\n");
            return;
        }

        if (c == CTRL_S) {
            fat_write_file(filename, buffer, file_len);
            kprint("\n[File Saved to Disk!]\n");
            sound_beep(1200, 150);
            return;
        }

        if (c == '\b') {
            if (cursor_idx > 0) {
                for (uint32_t i = cursor_idx - 1; i < file_len - 1; i++) {
                    buffer[i] = buffer[i + 1];
                }
                file_len--;
                buffer[file_len] = '\0';
                cursor_idx--;
                render_nano(filename, buffer, file_len, is_replace_mode, cursor_idx);
            }
            continue;
        }

        if ((unsigned char)c < 0x80) {
            if (is_replace_mode && cursor_idx < file_len) {
                buffer[cursor_idx] = c;
            } else {
                if (file_len < FILE_SIZE - 1) {
                    for (uint32_t i = file_len; i > cursor_idx; i--) {
                        buffer[i] = buffer[i - 1];
                    }
                    buffer[cursor_idx] = c;
                    file_len++;
                }
            }
            cursor_idx++;
            render_nano(filename, buffer, file_len, is_replace_mode, cursor_idx);
        }
    }
}

void execute_command(const char* cmd) {
    if (strcmp(cmd, "HELP") == 0 || strcmp(cmd, "help") == 0) {
        kprint("Commands:\n");
        kprint("  CF <file>       - Create File\n");
        kprint("  CDIR <path>     - Create Directory\n");
        kprint("  CD <path>       - Change Directory\n");
        kprint("  RD <path>       - Remove Directory\n");
        kprint("  RF <file>       - Remove File\n");
        kprint("  NANO <file>     - Edit file\n");
        kprint("  LS / DIR        - List Directory Contents\n");
        kprint("  CLS             - Clear Screen\n");
        kprint("  PLAY            - Play music\n");
    }
    else if (strncmp(cmd, "CF ", 3) == 0 || strncmp(cmd, "cf ", 3) == 0) {
        fat_create_file(cmd + 3);
    }
    else if (strncmp(cmd, "CDIR ", 5) == 0 || strncmp(cmd, "cdir ", 5) == 0) {
        fat_mkdir(cmd + 5);
    }
    else if (strncmp(cmd, "CD ", 3) == 0 || strncmp(cmd, "cd ", 3) == 0) {
        fat_cd(cmd + 3);
    }
    else if (strncmp(cmd, "RD ", 3) == 0 || strncmp(cmd, "rd ", 3) == 0) {
        fat_rmdir(cmd + 3);
    }
    else if (strncmp(cmd, "RF ", 3) == 0 || strncmp(cmd, "rf ", 3) == 0) {
        fat_remove_file(cmd + 3);
    }
    else if (strncmp(cmd, "NANO ", 5) == 0 || strncmp(cmd, "nano ", 5) == 0) {
        nano_editor(cmd + 5);
        init_registry();
        kclear_screen();
    }
    else if (strcmp(cmd, "LS") == 0 || strcmp(cmd, "ls") == 0 || strcmp(cmd, "DIR") == 0 || strcmp(cmd, "dir") == 0) {
        fat_list_directory();
    }
    else if (strcmp(cmd, "CLS") == 0 || strcmp(cmd, "cls") == 0) {
        kclear_screen();
    }
    else if (strcmp(cmd, "PLAY") == 0 || strcmp(cmd, "play") == 0) {
        kprint("Playing music...\n");
        play_melody();
    }
    else if (cmd[0] != '\0') {
        kprint("Unknown command: "); kprint(cmd); kprint("\n");
    }
}

void kernel_main() {
    idt_init();
    fat_init();
    
    init_registry();
    kclear_screen();

    sound_beep(900, 150);

    kprint("=== Bare-Metal Operating System ===\n\n");

    char cmd_buffer[CMD_BUFFER_SIZE];
    size_t cmd_length = 0;

    while (1) {
        fat_pwd();
        kprint("> ");

        while (1) {
            char c = keyboard_read_char();

            if (c == CTRL_C) {
                kprint("^C\n");
                cmd_length = 0;
                break;
            }

            if (c == CTRL_Z) {
                kprint("^Z\n");
                cmd_length = 0;
                break;
            }

            if (c == '\n') {
                kprint_char('\n');
                cmd_buffer[cmd_length] = '\0';
                execute_command(cmd_buffer);
                cmd_length = 0;
                break;
            } else if (c == '\b') {
                if (cmd_length > 0) {
                    cmd_length--;
                    kprint_char('\b');
                }
            } else if ((unsigned char)c < 0x80) {
                if (cmd_length < CMD_BUFFER_SIZE - 1) {
                    cmd_buffer[cmd_length++] = c;
                    kprint_char(c);
                }
            }
        }
    }
}