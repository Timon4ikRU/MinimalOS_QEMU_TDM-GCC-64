#include "fat.h"
#include "vga.h"
#include "ata.h"

// Sorry for no comments in the code

#define MAX_NODES 64
#define MAX_PATH 128
#define MAX_DATA 512
#define DISK_LBA_SECTOR 2

typedef enum { NODE_FILE, NODE_DIR } node_type_t;

typedef struct {
    char name[32];
    char parent_path[MAX_PATH];
    node_type_t type;
    char data[MAX_DATA];
    uint32_t size;
    uint8_t used;
} vfs_node_t;

static vfs_node_t nodes[MAX_NODES];
static char current_dir[MAX_PATH] = "/";

void strcpy(char* dest, const char* src) {
    while ((*dest++ = *src++));
}

void strcat(char* dest, const char* src) {
    while (*dest) dest++;
    while ((*dest++ = *src++));
}

int strlen(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

static void save_vfs_to_disk(void) {
    ata_write_sector(DISK_LBA_SECTOR, (const uint16_t*)nodes);
}

static void load_vfs_from_disk(void) {
    ata_read_sector(DISK_LBA_SECTOR, (uint16_t*)nodes);
}

static vfs_node_t* find_node(const char* parent, const char* name) {
    for (int i = 0; i < MAX_NODES; i++) {
        if (nodes[i].used && strcmp(nodes[i].parent_path, parent) == 0 && strcmp(nodes[i].name, name) == 0) {
            return &nodes[i];
        }
    }
    return 0;
}

void fat_init(void) {
    load_vfs_from_disk();
    if (nodes[0].used == 0) {
        fat_mkdir("sys");
        save_vfs_to_disk();
    }
}

void fat_pwd(void) {
    kprint(current_dir);
}

void fat_mkdir(const char* dir_name) {
    if (find_node(current_dir, dir_name)) return;
    for (int i = 0; i < MAX_NODES; i++) {
        if (!nodes[i].used) {
            strcpy(nodes[i].name, dir_name);
            strcpy(nodes[i].parent_path, current_dir);
            nodes[i].type = NODE_DIR;
            nodes[i].used = 1;
            save_vfs_to_disk();
            return;
        }
    }
}

void fat_cd(const char* dir_name) {
    if (strcmp(dir_name, "..") == 0) {
        if (strcmp(current_dir, "/") == 0) return;
        int len = strlen(current_dir);
        if (current_dir[len - 1] == '/') current_dir[len - 1] = '\0';
        for (int i = strlen(current_dir) - 1; i >= 0; i--) {
            if (current_dir[i] == '/') {
                current_dir[i + 1] = '\0';
                break;
            }
        }
        if (strlen(current_dir) == 0) strcpy(current_dir, "/");
        return;
    }

    vfs_node_t* node = find_node(current_dir, dir_name);
    if (node && node->type == NODE_DIR) {
        if (strcmp(current_dir, "/") != 0) strcat(current_dir, "/");
        strcat(current_dir, dir_name);
    } else {
        kprint("Directory not found: ");
        kprint(dir_name);
        kprint("\n");
    }
}

void fat_rmdir(const char* dir_name) {
    vfs_node_t* node = find_node(current_dir, dir_name);
    if (node && node->type == NODE_DIR) {
        node->used = 0;
        save_vfs_to_disk();
    }
}

void fat_create_file(const char* filename) {
    if (find_node(current_dir, filename)) return;
    for (int i = 0; i < MAX_NODES; i++) {
        if (!nodes[i].used) {
            strcpy(nodes[i].name, filename);
            strcpy(nodes[i].parent_path, current_dir);
            nodes[i].type = NODE_FILE;
            nodes[i].size = 0;
            nodes[i].used = 1;
            save_vfs_to_disk();
            return;
        }
    }
}

void fat_remove_file(const char* filename) {
    vfs_node_t* node = find_node(current_dir, filename);
    if (node && node->type == NODE_FILE) {
        node->used = 0;
        save_vfs_to_disk();
    }
}

void fat_write_file(const char* filepath, const char* buffer, uint32_t length) {
    vfs_node_t* node = find_node(current_dir, filepath);
    if (!node) {
        fat_create_file(filepath);
        node = find_node(current_dir, filepath);
    }
    if (node) {
        for (uint32_t i = 0; i < length && i < MAX_DATA; i++) node->data[i] = buffer[i];
        node->size = length;
        save_vfs_to_disk();
    }
}

int fat_read_file(const char* filepath, char* buffer, uint32_t max_len) {
    vfs_node_t* node = find_node(current_dir, filepath);
    if (node && node->type == NODE_FILE) {
        uint32_t i = 0;
        for (; i < node->size && i < max_len; i++) buffer[i] = node->data[i];
        buffer[i] = '\0';
        return i;
    }
    return 0;
}

void fat_list_directory(void) {
    kprint("Directory of ");
    kprint(current_dir);
    kprint("\n\n");
    for (int i = 0; i < MAX_NODES; i++) {
        if (nodes[i].used && strcmp(nodes[i].parent_path, current_dir) == 0) {
            if (nodes[i].type == NODE_DIR) {
                kprint("<DIR>          ");
            } else {
                kprint("<FILE>         ");
            }
            kprint(nodes[i].name);
            kprint("\n");
        }
    }
}