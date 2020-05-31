#include "ww_zipfs.h"

#include "zlib/contrib/minizip/unzip.h"

static unzFile ww_zipfs_file = NULL;

int ww_zipfs_init(char const* const zip_path) {
    ww_zipfs_file = unzOpen(zip_path);
    return ww_zipfs_file != NULL ? 0 : -1;
}

void ww_zipfs_destroy(void) {
    unzClose(ww_zipfs_file);
    ww_zipfs_file = NULL;
}

void* ww_zipfs_read(char const* file_path, size_t* const size) {
    if (ww_zipfs_file == NULL) {
        return NULL;
    }

    if (unzLocateFile(ww_zipfs_file, file_path, 1) != UNZ_OK) {
	    return NULL;
    }

    unz_file_info info;

    if (unzGetCurrentFileInfo(ww_zipfs_file, &info, NULL, 0, NULL, 0, NULL, 0) != UNZ_OK) {
	    return NULL;
    }

    if (unzOpenCurrentFile(ww_zipfs_file) != UNZ_OK) {
    	return NULL;
    }

    void* const buffer = malloc(info.uncompressed_size);

    if (buffer == NULL) {
        unzCloseCurrentFile(ww_zipfs_file);
        return NULL;
    }

    if (unzReadCurrentFile(ww_zipfs_file, buffer, info.uncompressed_size) != info.uncompressed_size) {
        free(buffer);
        unzCloseCurrentFile(ww_zipfs_file);
        return NULL;
    }

    unzCloseCurrentFile(ww_zipfs_file);
    *size = info.uncompressed_size;
    return buffer;
}
