#ifndef WW_FILESYS_H
#define WW_FILESYS_H

#include <stdlib.h>
#include <stdio.h>

int  ww_filesys_init(char const* path);
void ww_filesys_destroy(void);

int   ww_filesys_exists(char const* file_path);
void* ww_filesys_read_all(char const* file_path, size_t* size);

#ifndef NDEBUG
typedef void (*ww_filesys_traverse_t)(char const* path, int is_tar);
void ww_filesys_traverse(ww_filesys_traverse_t callback);
#endif

#endif // WW_FILESYS_H
