#ifndef WW_ZIPFS_H
#define WW_ZIPFS_H

#include <stdlib.h>

int  ww_zipfs_init(char const* zip_path);
void ww_zipfs_destroy(void);

void* ww_zipfs_read(char const* file_path, size_t* size);

#endif /* WW_ZIPFS_H */
