#include "ww_tile.h"
#include "ww_filesys.h"
#include "ww_config.h"

#include <png.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef uint8_t ww_tile_t[WW_TILE_SIZE][WW_TILE_SIZE];

static ww_tile_t ww_tiles[WW_MAX_TILES];

// https://lospec.com/palette-list/aap-64
static png_color ww_tile_palette[] = {
    {0x00, 0x00, 0x00}, // Palette entry 0 is transparent.
    {0x06, 0x06, 0x08}, {0x14, 0x10, 0x13}, {0x3b, 0x17, 0x25}, {0x73, 0x17, 0x2d},
    {0xb4, 0x20, 0x2a}, {0xdf, 0x3e, 0x23}, {0xfa, 0x6a, 0x0a}, {0xf9, 0xa3, 0x1b},
    {0xff, 0xd5, 0x41}, {0xff, 0xfc, 0x40}, {0xd6, 0xf2, 0x64}, {0x9c, 0xdb, 0x43},
    {0x59, 0xc1, 0x35}, {0x14, 0xa0, 0x2e}, {0x1a, 0x7a, 0x3e}, {0x24, 0x52, 0x3b},
    {0x12, 0x20, 0x20}, {0x14, 0x34, 0x64}, {0x28, 0x5c, 0xc4}, {0x24, 0x9f, 0xde},
    {0x20, 0xd6, 0xc7}, {0xa6, 0xfc, 0xdb}, {0xff, 0xff, 0xff}, {0xfe, 0xf3, 0xc0},
    {0xfa, 0xd6, 0xb8}, {0xf5, 0xa0, 0x97}, {0xe8, 0x6a, 0x73}, {0xbc, 0x4a, 0x9b},
    {0x79, 0x3a, 0x80}, {0x40, 0x33, 0x53}, {0x24, 0x22, 0x34}, {0x22, 0x1c, 0x1a},
    {0x32, 0x2b, 0x28}, {0x71, 0x41, 0x3b}, {0xbb, 0x75, 0x47}, {0xdb, 0xa4, 0x63},
    {0xf4, 0xd2, 0x9c}, {0xda, 0xe0, 0xea}, {0xb3, 0xb9, 0xd1}, {0x8b, 0x93, 0xaf},
    {0x6d, 0x75, 0x8d}, {0x4a, 0x54, 0x62}, {0x33, 0x39, 0x41}, {0x42, 0x24, 0x33},
    {0x5b, 0x31, 0x38}, {0x8e, 0x52, 0x52}, {0xba, 0x75, 0x6a}, {0xe9, 0xb5, 0xa3},
    {0xe3, 0xe6, 0xff}, {0xb9, 0xbf, 0xfb}, {0x84, 0x9b, 0xe4}, {0x58, 0x8d, 0xbe},
    {0x47, 0x7d, 0x85}, {0x23, 0x67, 0x4e}, {0x32, 0x84, 0x64}, {0x5d, 0xaf, 0x8d},
    {0x92, 0xdc, 0xba}, {0xcd, 0xf7, 0xe2}, {0xe4, 0xd2, 0xaa}, {0xc7, 0xb0, 0x8b},
    {0xa0, 0x86, 0x62}, {0x79, 0x67, 0x55}, {0x5a, 0x4e, 0x44}, {0x42, 0x39, 0x34}
};

int ww_tile_init(void) {
    return 0;
}

void ww_tile_destroy(void) {
}

static void ww_tile_png_error(png_structp const png, png_const_charp const error) {
    (void)png;
    (void)error;
}

typedef struct {
    uint8_t const* buffer;
    size_t remaining;
}
ww_tile_reader_t;

static void ww_tile_png_read(png_structp const png, png_bytep const buffer, size_t const size) {
    ww_tile_reader_t* const reader = png_get_io_ptr(png);
    size_t const to_read = size <= reader->remaining ? size : reader->remaining;

    memcpy(buffer, reader->buffer, to_read);
    reader->buffer += to_read;
    reader->remaining -= to_read;
}

int ww_tile_load(uint8_t const tile_num, char const* const file_path) {
    if (tile_num >= WW_MAX_TILES) {
        return -1;
    }

    void* buffer;
    ww_tile_reader_t reader;
    reader.buffer = buffer = ww_filesys_read_all(file_path, &reader.remaining);

    if (reader.buffer == NULL) {
        return -1;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, ww_tile_png_error, ww_tile_png_error);

    if (png == NULL) {
error1:
        free(buffer);
        return -1;
    }

    png_infop info = png_create_info_struct(png);

    if (info == NULL) {
        png_destroy_read_struct(&png, NULL, NULL);
        goto error1;
    }

    if (setjmp(png_jmpbuf(png))) {
error2:
        png_destroy_read_struct(&png, &info, NULL);
        goto error1;
    }

    png_set_read_fn(png, &reader, ww_tile_png_read);
    png_read_info(png, info);

    png_uint_32 width = png_get_image_width(png, info);
    png_uint_32 height = png_get_image_height(png, info);

    if (width != WW_TILE_SIZE || height != WW_TILE_SIZE) {
        goto error2;
    }

    // Make sure we always get RGBA pixels.
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    if (bit_depth == 16) {
        png_set_strip_16(png);
    }

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png);
    }

    // Transform transparent color to alpha.
    if (png_get_valid(png, info, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png);
    }

    // Set alpha to opaque if non-existent.
    if (color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE) {

        png_set_filler(png, 0xff, PNG_FILLER_AFTER);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png);
    }

    png_read_update_info(png, info);

    uint32_t const pixels[WW_TILE_SIZE][WW_TILE_SIZE];
    png_bytep rows[WW_TILE_SIZE];
    
    for (size_t i = 0; i < WW_TILE_SIZE; i++) {
        rows[i] = (png_bytep)pixels[i];
    }
    
    png_read_image(png, rows);
    free((void*)buffer);
    png_destroy_read_struct(&png, &info, NULL);

    ww_tile_t* tile = ww_tiles + tile_num;

    for (unsigned y = 0; y < WW_TILE_SIZE; y++) {
        for (unsigned x = 0; x < WW_TILE_SIZE; x++) {
            uint32_t const color = pixels[y][x];

            if (((color >> 24) & 0xff) == 0) {
                // Transparent.
                (*tile)[y][x] = 0;
                continue;
            }

            float const r = color & 0xff;
            float const g = (color >> 8) & 0xff;
            float const b = (color >> 16) & 0xff;

            float dr = r - ww_tile_palette[1].red;
            float dg = g - ww_tile_palette[1].green;
            float db = b - ww_tile_palette[1].blue;

            float min_distance = dr * dr + dg * dg + db * db;
            size_t min_index = 1;

            for (size_t i = 2; i < sizeof(ww_tile_palette) / sizeof(ww_tile_palette[0]); i++) {
                dr = r - ww_tile_palette[i].red;
                dg = g - ww_tile_palette[i].green;
                db = b - ww_tile_palette[i].blue;

                float distance = dr * dr + dg * dg + db * db;

                if (distance < min_distance) {
                    min_distance = distance;
                    min_index = i;
                }
            }

            (*tile)[y][x] = (uint8_t)min_index;
        }
    }

    return 0;
}

void ww_tile_blit(uint32_t* canvas, uint8_t const tile_num, int x0, int y0) {
    int width = WW_TILE_SIZE;
    int ox = 0;

    if (x0 < 0) {
        width += x0;
        ox -= x0;
        x0 = 0;
    }
    else if (x0 + WW_TILE_SIZE > WW_SCREEN_WIDTH) {
        width -= x0 + WW_TILE_SIZE - WW_SCREEN_WIDTH;
    }

    if (width <= 0) {
        return;
    }

    int height = WW_TILE_SIZE;
    int oy = 0;

    if (y0 < 0) {
        height += y0;
        oy -= y0;
        y0 = 0;
    }
    else if (y0 + WW_TILE_SIZE > WW_SCREEN_HEIGHT) {
        height -= y0 + WW_TILE_SIZE - WW_SCREEN_HEIGHT;
    }

    if (height <= 0) {
        return;
    }

    canvas += y0 * WW_SCREEN_WIDTH + x0;
    ww_tile_t* tile = ww_tiles + tile_num;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++, canvas++) {
            uint8_t const index = (*tile)[oy + y][ox + x];

            if (index != 0) {
                uint32_t r = ww_tile_palette[index].red;
                uint32_t g = ww_tile_palette[index].green;
                uint32_t b = ww_tile_palette[index].blue;

                *canvas = r << 16 | g << 8 | b;
            }
        }

        canvas += WW_SCREEN_WIDTH - width;
    }
}
