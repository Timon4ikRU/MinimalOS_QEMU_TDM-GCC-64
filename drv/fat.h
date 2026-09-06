#ifndef FAT_H
#define FAT_H

#include <stdint.h>
#include <stddef.h>

int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);

void fat_init(void);
void fat_cd(const char* dir_name);
void fat_pwd(void);
void fat_mkdir(const char* dir_name);
void fat_rmdir(const char* dir_name);
void fat_create_file(const char* filename);
void fat_remove_file(const char* filename);
void fat_write_file(const char* filepath, const char* buffer, uint32_t length);
int  fat_read_file(const char* filepath, char* buffer, uint32_t max_len);
void fat_list_directory(void);

#endif