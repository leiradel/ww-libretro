#include "ww_filesys.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

typedef union {
    struct {
        uint8_t name[100];
        uint8_t mode[8];
        uint8_t owner[8];
        uint8_t group[8];
        uint8_t size[12];
        uint8_t modification[12];
        uint8_t checksum[8];
        uint8_t type;
        uint8_t linked[100];
    }
    s;

    uint8_t fill[512];
}
ww_filesys_tarentry_t;

typedef struct {
    union {
        FILE* tar;
        char const* fs_path;
    }
    u;

    long offset;
    long size;
    char const* mp_path;
}
ww_filesys_entry_t;

typedef int (*ww_filesys_folder_callback_t)(char const*, char const*);
typedef int (*ww_filesys_tar_callback_t)(FILE* tar, long, long, char const*);

static struct {
    ww_filesys_entry_t* elements;
    size_t count, reserved;
} ww_filesys_entries;

static struct {
    FILE** elements;
    size_t count, reserved;
} ww_filesys_fps;

static int ww_filesys_tar_entry(FILE* const tar,
                                long const offset,
                                long const size,
                                char const* const mount_path);

static char* ww_filesys_strdup(char const* const str) {
    size_t const length = strlen(str);
    char* const copy = (char*)malloc(length + 1);

    if (copy != NULL) {
        memcpy(copy, str, length + 1);
    }

    return copy;
}

static ww_filesys_entry_t* ww_filesys_add_entry(void) {
    if (ww_filesys_entries.count == ww_filesys_entries.reserved) {
        size_t const new_reserved = ww_filesys_entries.reserved == 0
                                  ? 128
                                  : ww_filesys_entries.reserved * 2;

        ww_filesys_entry_t* const new_entries =
            (ww_filesys_entry_t*)realloc(ww_filesys_entries.elements, new_reserved * sizeof(*new_entries));
        
        if (new_entries == NULL) {
            return NULL;
        }

        ww_filesys_entries.elements = new_entries;
        ww_filesys_entries.reserved = new_reserved;
    }

    return ww_filesys_entries.elements + ww_filesys_entries.count++;
}

static FILE** ww_filesys_add_fps(void) {
    if (ww_filesys_fps.count == ww_filesys_fps.reserved) {
        size_t const new_reserved = ww_filesys_fps.reserved == 0
                                  ? 128
                                  : ww_filesys_fps.reserved * 2;

        FILE** const new_entries =
            (FILE**)realloc(ww_filesys_fps.elements, new_reserved * sizeof(*new_entries));
        
        if (new_entries == NULL) {
            return NULL;
        }

        ww_filesys_fps.elements = new_entries;
        ww_filesys_fps.reserved = new_reserved;
    }

    return ww_filesys_fps.elements + ww_filesys_fps.count++;
}

static int ww_filesys_traverse_folder(char const* const folder_path,
                                      char const* const mount_path,
                                      ww_filesys_folder_callback_t callback) {

    DIR* const dir = opendir(folder_path);

    if (dir == NULL) {
        return -1;
    }

    for (;;) {
        struct dirent const* const dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            closedir(dir);
            return 0;
        }

        size_t const file_path_len = strlen(folder_path) + strlen(dir_entry->d_name) + 2;
        char* const file_path = (char*)malloc(file_path_len);

        if (file_path == NULL) {
    error1:
            closedir(dir);
            return -1;
        }

        size_t const new_mount_path_len = strlen(mount_path) + strlen(dir_entry->d_name) + 2;
        char* const new_mount_path = (char*)malloc(new_mount_path_len);

        if (new_mount_path == NULL) {
    error2:
            free(file_path);
            goto error1;
        }

        snprintf(file_path, file_path_len, "%s/%s", folder_path, dir_entry->d_name);
        snprintf(new_mount_path, new_mount_path_len, "%s/%s", mount_path, dir_entry->d_name);

        struct stat stbuf;

        if (stat(file_path, &stbuf) != 0) {
    error3:
            free(new_mount_path);
            goto error2;
        }

        if (S_ISREG(stbuf.st_mode)) {
            if (callback(file_path, new_mount_path) != 0) {
                goto error3;
            }
        }
        else if (S_ISDIR(stbuf.st_mode)) {
            int const dot1 = dir_entry->d_name[0] == '.';
            int const is_self = dot1 && dir_entry->d_name[1] == 0;
            int const dot2 = dot1 && dir_entry->d_name[1] == '.';
            int const is_parent = dot2 && dir_entry->d_name[2] == 0;

            if (!is_self && !is_parent) {
                if (ww_filesys_traverse_folder(file_path, new_mount_path, callback) != 0) {
                    goto error3;
                }
            }
        }

        free(new_mount_path);
        free(file_path);
    }
}

static int ww_filesys_traverse_tar(FILE* const tar,
                                   long const offset,
                                   long const size,
                                   char const* const mount_path,
                                   ww_filesys_tar_callback_t callback) {

    if ((size & 511) != 0) {
        return -1;
    }

    for (long position = 0; position < size; position += 512) {
        ww_filesys_tarentry_t entry;

        if (fseek(tar, position, SEEK_SET) != 0) {
            return -1;
        }

        if (fread(&entry, 1, 512, tar) != 512) {
            return -1;
        }

        if (entry.s.name[0] == 0) {
            return 0;
        }

        long const size = strtol((char*)entry.s.size, NULL, 8);

        if (size == 0) {
            continue; // It's a folder
        }

        for (size_t i = sizeof(entry.s.name) - 1;; i--) {
            if (i == 1 || entry.s.name[i] != ' ') {
                size_t const new_mount_path_len = strlen(mount_path) + i + 2;
                char* const new_mount_path = (char*)malloc(new_mount_path_len);

                if (new_mount_path == NULL) {
                    return -1;
                }

                snprintf(new_mount_path, new_mount_path_len, "%s/%s", mount_path, entry.s.name);
                long const offset = position + 512;
                position += (size + 511) & ~511;

                if (callback(tar, offset, size, new_mount_path) != 0) {
                    free(new_mount_path);
                    return -1;
                }

                free(new_mount_path);
                break;
            }
        }
    }

    return -1;
}

static int ww_filesys_mount_tar_file(char const* const tar_path,
                                     char const* const mount_path,
                                     ww_filesys_tar_callback_t callback) {

    struct stat stbuf;

    if (stat(tar_path, &stbuf) != 0) {
        return -1;
    }

    FILE** const tar = ww_filesys_add_fps();

    if (tar == NULL) {
        return -1;
    }

    *tar = fopen(tar_path, "rb");

    if (ww_filesys_traverse_tar(*tar, 0, stbuf.st_size, mount_path, callback)) {
        return -1;
    }

    return 0;
}

static int ww_filesys_fs_entry(char const* const file_path,
                               char const* const mount_path) {

    char const* const ext = strstr(mount_path, ".ww");

    if (ext != NULL && (ext[3] == 'm' || ext[3] == 'l' || ext[3] == 'g') && ext[4] == 0) {
        // It's a tar file with a microgame, a level, or a game
        return ww_filesys_mount_tar_file(file_path, mount_path, ww_filesys_tar_entry);
    }

    char* const fs_path = ww_filesys_strdup(file_path);

    if (fs_path == NULL) {
        return -1;
    }

    char* const mp_path = ww_filesys_strdup(mount_path);

    if (mp_path == NULL) {
        free(fs_path);
        return -1;
    }

    ww_filesys_entry_t* const entry = ww_filesys_add_entry();

    if (entry == NULL) {
        free(mp_path);
        free(fs_path);
        return -1;
    }

    entry->u.fs_path = fs_path;
    entry->offset = 0;
    entry->size = 0;
    entry->mp_path = mp_path;

    return 0;
}

static int ww_filesys_tar_entry(FILE* const tar,
                                long const offset,
                                long const size,
                                char const* const mount_path) {

    char const* const ext = strstr(mount_path, ".ww");

    if (ext != NULL && (ext[3] == 'm' || ext[3] == 'l' || ext[3] == 'g') && ext[4] == 0) {
        // It's a tar file with a microgame, a level, or a game
        return ww_filesys_traverse_tar(tar, offset, size, mount_path, ww_filesys_tar_entry);
    }

    char* const mp_path = ww_filesys_strdup(mount_path);

    if (mp_path == NULL) {
        return -1;
    }

    ww_filesys_entry_t* const entry = ww_filesys_add_entry();

    if (entry == NULL) {
        free(mp_path);
        return -1;
    }

    entry->u.tar = tar;
    entry->offset = offset;
    entry->size = size;
    entry->mp_path = mp_path;

    return 0;
}

static int ww_filesys_compare(void const* v1, void const* v2) {
    ww_filesys_entry_t const* e1 = (ww_filesys_entry_t const*)v1;
    ww_filesys_entry_t const* e2 = (ww_filesys_entry_t const*)v2;

    return strcmp(e1->mp_path, e2->mp_path);
}

int ww_filesys_init(char const* const path) {
    ww_filesys_entries.elements = NULL;
    ww_filesys_entries.count = 0;
    ww_filesys_entries.reserved = 0;

    ww_filesys_fps.elements = NULL;
    ww_filesys_fps.count = 0;
    ww_filesys_fps.reserved = 0;

    struct stat stbuf;

    if (stat(path, &stbuf) != 0) {
        return -1;
    }

    int res = -1;

    if (S_ISDIR(stbuf.st_mode)) {
        res = ww_filesys_traverse_folder(path, "", ww_filesys_fs_entry);
    }
    else {
        res = ww_filesys_mount_tar_file(path, "", ww_filesys_tar_entry);
    }

    if (res != 0) {
        ww_filesys_destroy();
        return -1;
    }

    ww_filesys_entry_t* const elements = ww_filesys_entries.elements;
    size_t const count = ww_filesys_entries.count;
    qsort(elements, count, sizeof(ww_filesys_entry_t), ww_filesys_compare);
    return 0;
}

void ww_filesys_destroy(void) {
    for (size_t i = 0; i < ww_filesys_entries.count; i++) {
        free((void*)ww_filesys_entries.elements[i].mp_path);

        if (ww_filesys_entries.elements[i].size == 0) {
            free((void*)ww_filesys_entries.elements[i].u.fs_path);
        }
    }

    for (size_t i = 0; i < ww_filesys_fps.count; i++) {
        fclose(ww_filesys_fps.elements[i]);
    }

    free(ww_filesys_entries.elements);
    free(ww_filesys_fps.elements);
}

int ww_filesys_exists(char const* file_path) {
    ww_filesys_entry_t key;
    key.mp_path = file_path;

    ww_filesys_entry_t const* const elements = ww_filesys_entries.elements;
    size_t const count = ww_filesys_entries.count;
    return bsearch(&key, elements, count, sizeof(ww_filesys_entry_t), ww_filesys_compare) != NULL;
}

void* ww_filesys_read_all(char const* file_path, size_t* const size) {
    ww_filesys_entry_t key;
    key.mp_path = file_path;

    ww_filesys_entry_t const* const elements = ww_filesys_entries.elements;
    size_t const count = ww_filesys_entries.count;
    ww_filesys_entry_t* const found = bsearch(&key, elements, count, sizeof(ww_filesys_entry_t), ww_filesys_compare);

    if (found == NULL) {
        return NULL;
    }

    if (found->size == 0) {
        // File system
        struct stat stbuf;

        if (stat(found->u.fs_path, &stbuf) != 0) {
            return NULL;
        }

        void* const buffer = malloc(stbuf.st_size);

        if (buffer == NULL) {
            return NULL;
        }

        FILE* const fp = fopen(found->u.fs_path, "rb");

        if (fp == NULL) {
            free(buffer);
            return NULL;
        }

        if (fread(buffer, 1, stbuf.st_size, fp) != stbuf.st_size) {
            fclose(fp);
            free(buffer);
            return NULL;
        }

        fclose(fp);
        *size = stbuf.st_size;
        return buffer;
    }
    else {
        // TAR entry.
        if (fseek(found->u.tar, found->offset, SEEK_SET) != 0) {
            return NULL;
        }

        void* const buffer = malloc(found->size);

        if (buffer == NULL) {
            return NULL;
        }

        if (fread(buffer, 1, found->size, found->u.tar) != found->size) {
            free(buffer);
            return NULL;
        }

        *size = found->size;
        return buffer;
    }
}

#ifndef NDEBUG
void ww_filesys_traverse(ww_filesys_traverse_t callback) {
    for (size_t i = 0; i < ww_filesys_entries.count; i++) {
        callback(ww_filesys_entries.elements[i].mp_path, ww_filesys_entries.elements[i].size != 0);
    }
}
#endif
